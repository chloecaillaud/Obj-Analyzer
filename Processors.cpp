#pragma once
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>
#include <map>
#include <optional>
#include <chrono>

#include "Settings.cpp"
#include "LogManager.cpp"
using namespace std;
namespace fs = std::filesystem;

class LineProcessor_Obj
{
public:
	string lineStr;

	string_view getLineType() const
	{
		return string_view(lineStr.data(), lineStr.find(' '));
	}

	uint32_t getValueCount() const
	{
		uint32_t valueCount = 0;
		for (size_t i = 0; i < lineStr.size() - 1; i++)
		{
			// all values are prefixed with a space
			// therfore space count = values count (unless malformed file)
			if (lineStr[i] == ' ') { valueCount++; }
		}
		return valueCount;
	}

	vector<string_view> getValues() const
	{
		vector<string_view> values;

		size_t lastDelimiter = 0;
		for (size_t i = 0; i < lineStr.size(); i++)
		{
			if (lineStr[i] == ' ')
			{
				// skip first delim since represent line type not value
				if (lastDelimiter != 0)
				{
					values.push_back(string_view(
						lineStr.data() + lastDelimiter,
						lineStr.data() + i
					));
				}
				lastDelimiter = i;
			}
		}
		// add last value since no delim at eol
		if (lastDelimiter != 0)
		{
			values.push_back(string_view(
				lineStr.data() + lastDelimiter,
				lineStr.data() + lineStr.size()
			));
		}
		return values;
	}
};

class FileProcessor_Obj
{
protected:
	const fs::path& filepath;
	LogManager& loggingManager;
	uint64_t lineNum = 0;

public:
	const ProcessingSettings& settings;
	vector<AssetData> assets;

	FileProcessor_Obj(const fs::path& filepath, const ProcessingSettings& settings, LogManager& loggingManager)
		: filepath(filepath)
		, settings(settings)
		, loggingManager(loggingManager)
		, assets({ AssetData(filepath.stem().string()) }) //default obj for malformed file lacking "g" or "o" lines or when settings mode:File
	{};

	const fs::path& getFilepath() const
	{
		return filepath;
	}

	AssetData& getCurrentObject()
	{
		return assets.back();
	}

	void processFile()
	{
		loggingManager.logProcessingMsg(ProcessingLogs::ProcessingStart_log);
		loggingManager.logIoMsg(IoLogs::InPathRead_log, filepath);
		auto start_time = chrono::system_clock::now();

		ifstream file(filepath, ios::binary);
		LineProcessor_Obj lineProcessor;

		lineNum = 0;
		while (getline(file, lineProcessor.lineStr))
		{
			lineNum++;
			const string_view lineType = lineProcessor.getLineType();
			// prim lineType
			if (lineType == "v" && settings.areVertsRelevent())
			{
				AssetData& currObj = getCurrentObject();
				currObj.vertCount++;

				if (currObj.hasVertColor == false) // avoid getting values when not needed
				{
					if (lineProcessor.getValueCount() == 6)
					{
						// verts with color are specified like: x y z r g b
						// NOTE: color support is technically unofficial and may differ from author to author *cough* *cough* Zbrush
						currObj.hasVertColor = true;
					}
				}
			}
			else if (lineType == "vn" && settings.areVertsRelevent())
			{
				getCurrentObject().hasVertNormals = true;
			}
			else if (lineType == "vt" && settings.areVertsRelevent())
			{
				getCurrentObject().hasUvs = true;
			}
			else if (lineType == "p" && settings.arePointsRelevent())
			{
				getCurrentObject().pointCount++;
			}
			else if (lineType == "l" && settings.areLinesRelevent())
			{
				getCurrentObject().lineCount++;
			}
			else if (lineType == "f" && settings.areFacesRelevent())
			{
				getCurrentObject().faceTotalCount++;
				const uint32_t vertCount = lineProcessor.getValueCount();
				switch (vertCount)
				{
				case 3:  getCurrentObject().faceTriCount++;  break;
				case 4:  getCurrentObject().faceQuadCount++; break;
				default: getCurrentObject().faceNgonCount++; break;
				}
			}
			// container lineType
			else if (lineType == "o")
			{
				const auto& lineValues = lineProcessor.getValues();
				if (settings.grouping == assetDataGrouping::Object)
				{
					// treat as asset grouping
					if (!lineValues.empty())
					{
						assets.push_back(AssetData{ lineValues.front() });
						if (lineValues.size() > 1)
						{
							loggingManager.logProcessingMsg(ProcessingLogs::ONameMoreThanOne_warn, lineNum);
						}
					}
					else
					{
						loggingManager.logProcessingMsg(ProcessingLogs::ONameMissing_warn, lineNum);
					}
				}
				else if (settings.grouping == assetDataGrouping::File && settings.areSubGroupsRelevent())
				{
					// treat as sub grouping
					getCurrentObject().subgroupCount++;
				}
			}
			else if (lineType == "g")
			{
				const auto& lineValues = lineProcessor.getValues();
				if (settings.grouping == assetDataGrouping::Vertexgroup)
				{
					// treat as asset grouping
					if (!lineValues.empty() && lineValues.front() != "off")
					{
						assets.push_back(AssetData{ lineValues.front() });
						if (lineValues.size() > 1)
						{
							loggingManager.logProcessingMsg(ProcessingLogs::GNameMoreThanOne_warn, lineNum);
						}
					}
					else if (lineValues.empty())
					{
						loggingManager.logProcessingMsg(ProcessingLogs::GNameMissing_warn, lineNum);
					}
				}
				else if (settings.grouping == assetDataGrouping::Object
					&& settings.areSubGroupsRelevent()
					&& lineValues.front() != "off"
					)
				{
					// treat as sub grouping
					getCurrentObject().subgroupCount++;
				}
			}
			// other
			else if (lineType == "usemtl" && settings.areMaterialsRelevent())
			{
				getCurrentObject().addMaterial(lineProcessor.getValues().front());
			}
		}
		file.close();

		if (!file.eof())
		{
			// failed before reaching end of file
			if (settings.multiFile())
			{
				loggingManager.logIoMsg(IoLogs::InFileReadFail_warn, filepath);
			}
			else
			{
				loggingManager.logIoMsg(IoLogs::InFileReadFail_err, filepath);
			}
		}
		// log elapsed time
		loggingManager.logProcessingMsg(ProcessingLogs::ProcessingEndStats_log,
			chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start_time),
			lineNum
			);
		loggingManager.logProcessingMsg(ProcessingLogs::ProcessingEnd_log);
	}
};

class CmdArgParcer
{
	LogManager& loggingManager;

	const map<const string_view, bool>& svToBoolTable
	{
		{ "true" , true },
		{ "t"    , true },
		{ "1"    , true },
		{ "false", false },
		{ "f"    , false },
		{ "0"    , false },
	};

	const map<string_view, ProcessingMode> svToModeTable
	{
		{ "o"        , ProcessingMode::Overview },
		{ "view"     , ProcessingMode::Overview },
		{ "overview" , ProcessingMode::Overview },
		{ "v"        , ProcessingMode::Validate },
		{ "valid"    , ProcessingMode::Validate },
		{ "validate" , ProcessingMode::Validate },
		{ "b"        , ProcessingMode::Budget },
		{ "budget"   , ProcessingMode::Budget },
	};

	const map<string_view,assetDataGrouping> svToGroupingTable
	{
		{ "f"           , assetDataGrouping::File },
		{ "file"        , assetDataGrouping::File },
		{ "o"           , assetDataGrouping::Object },
		{ "obj"         , assetDataGrouping::Object },
		{ "object"      , assetDataGrouping::Object },
		{ "g"           , assetDataGrouping::Vertexgroup },
		{ "vgroup"      , assetDataGrouping::Vertexgroup },
		{ "vertexgroup" , assetDataGrouping::Vertexgroup },
	};

	map<string_view, string_view> keywordArgs;
	vector<string_view> positionalArgs;

	bool convertSvToBool(string_view sv, bool defaultValue) const
	{
		auto it = svToBoolTable.find(sv);
		if (it != svToBoolTable.end())
		{
			return it->second;
		}
		else if (!sv.empty())
		{
			loggingManager.logProgramArgMsg(ProgramArgLogs::boolValueInvalid_warn, sv);
		}
		return defaultValue;
	}

	optional<string_view> getKargValue(string_view key) const
	{
		auto it = keywordArgs.find(key);
		if (it != keywordArgs.end())
		{
			return it->second;
		}
		return {};
	}

	void ProcessValidationSettings(ProcessingSettings& settings)
	{
		setValidationBoolElem(settings.validations.containsVerts,         "-v",     true);
		setValidationBoolElem(settings.validations.containsUvs,           "-uv",    true);
		setValidationBoolElem(settings.validations.containsVertexNormals, "-vn",    true);
		setValidationBoolElem(settings.validations.containsVertexColor,   "-vcol",  true);
		setValidationBoolElem(settings.validations.containsLooseEdges,    "-ledge", false);
		setValidationBoolElem(settings.validations.containsLoosePoints,   "-lpnt",  false);
		setValidationBoolElem(settings.validations.containsFaces,         "-f",     true);
		setValidationBoolElem(settings.validations.containsTris,          "-ft",    true);
		setValidationBoolElem(settings.validations.containsQuads,         "-fq",    true);
		setValidationBoolElem(settings.validations.containsNgons,         "-fng",   false);
		setValidationBoolElem(settings.validations.containsMaterials,     "-mat",   true);
		setValidationBoolElem(settings.validations.MissingName,           "-nn",    false);

		if (auto OptionalValue = getKargValue("-prefix"))
		{
			settings.validations.namePrefix.shouldCheck = true;
			settings.validations.namePrefix.substring = string(*OptionalValue);
		}

		if (auto OptionalValue = getKargValue("-suffix"))
		{
			settings.validations.nameSuffix.shouldCheck = true;
			settings.validations.nameSuffix.substring = string(*OptionalValue);
		}
	}

	void setValidationBoolElem(ValidationBoolElem& elem, string_view key, bool defaultExpectedValue)
	{
		if (auto OptionalValue = getKargValue(key))
		{
			elem.shouldCheck = true;
			elem.expectedValue = convertSvToBool(*OptionalValue, defaultExpectedValue);
		}
	}

	void ProcessBudgetSettings(ProcessingSettings& settings)
	{
		setBudgetUint32Elem(settings.budgets.verts,      "-v");
		setBudgetUint32Elem(settings.budgets.points,     "-p");
		setBudgetUint32Elem(settings.budgets.lines,      "-l");
		setBudgetUint32Elem(settings.budgets.faceTotals, "-f");
		setBudgetUint32Elem(settings.budgets.faceTris,   "-ft");
		setBudgetUint32Elem(settings.budgets.faceQuads,  "-fq");
		setBudgetUint32Elem(settings.budgets.faceNgons,  "-fng");
		setBudgetUint32Elem(settings.budgets.materials,  "-mat");
		setBudgetUint32Elem(settings.budgets.groups,     "-g");
	}

	void setBudgetUint32Elem(BudgetUint32Elem& elem, const string_view key)
	{
		if (auto OptionalValue = getKargValue(key))
		{
			elem.shouldCheck = true;

			//store in temp var since elem.value is actully a 31 bit field
			uint32_t resultValue;
			string_view& argValue = *OptionalValue;
			auto convertionErrStatus = from_chars(argValue.data(), argValue.data() + argValue.size(), resultValue).ec;
			if (convertionErrStatus == errc())
			{
				// no issues
				elem.value = resultValue;
			}
			else if (convertionErrStatus == errc::invalid_argument)
			{
				loggingManager.logProgramArgMsg(ProgramArgLogs::BudgetInvalid_err, key);
			}
			else if (convertionErrStatus == errc::result_out_of_range)
			{
				loggingManager.logProgramArgMsg(ProgramArgLogs::BudgetOutOfRange_warn, key, to_string(0x7FFFFFFF)); // max val for BudgetUint32Elem (2^31-1)
			}
		}
	}

	void getFilesFromDir(const fs::path& dirPath, vector<fs::path>& filepathCollection)
	{
		for (auto const& entry : fs::directory_iterator(dirPath, fs::directory_options::skip_permission_denied))
		{
			if (fs::is_regular_file(entry)
				&& entry.path().extension() == ".obj"
				)
			{
				filepathCollection.push_back(entry.path());
			}
		}
	}

public:
	CmdArgParcer(const int& argc,char* argv[], LogManager& loggingManager)
		: loggingManager(loggingManager)
	{
		string_view currentArgKey;
		for (int i = 1; i < argc; i++)
		{
			// 1 <= i <= argc-1 since 0 is prog name and argc is null term.
			if (argv[i][0] == '-')
			{
				// is keyword arg key
				currentArgKey = string_view(argv[i]);
				keywordArgs[currentArgKey];
			}
			else if (!currentArgKey.empty())
			{
				// keyword arg value
				keywordArgs[currentArgKey] = string_view(argv[i]);
			}
			else
			{
				// positional arg
				positionalArgs.push_back(string_view(argv[i]));
			}
		}
	}

	bool hasHelpArg() const
	{
		return getKargValue("-h").has_value();
	}

	fs::path getLogPath(bool logWarnings = true) const
	{
		if (auto OptionalValue = getKargValue("-log"))
		{
			fs::path logPath = *OptionalValue;
			if (!logPath.has_extension())
			{
				logPath += fs::path(".log");
			}

			if (logWarnings && fs::exists(logPath))
			{
				loggingManager.logIoMsg(IoLogs::LogFileAlreadyExists_warn, logPath);
			}

			return logPath;
		}
		return fs::path();
	}

	ProcessingSettings asSettings()
	{
		ProcessingSettings settings;

		// ---- req ----
		// obj file paths
		bool isMultiFile = positionalArgs.size() == 1;
		settings.inputFilePaths.reserve(positionalArgs.size());
		for (fs::path pathArg : positionalArgs)
		{
			if (fs::exists(pathArg))
			{
				if (fs::is_regular_file(pathArg))
				{
					if (pathArg.extension() == ".obj")
					{
						settings.inputFilePaths.push_back(move(fs::path(pathArg)));
					}
					else
					{
						loggingManager.logIoMsg(isMultiFile ? IoLogs::InFileInvalidType_warn : IoLogs::InFileInvalidType_err, pathArg);
					}
				}
				else if (fs::is_directory(pathArg))
				{
					getFilesFromDir(pathArg, settings.inputFilePaths);
				}
			}
			else
			{
				loggingManager.logIoMsg(isMultiFile ? IoLogs::InPathInvalid_warn : IoLogs::InPathInvalid_err, pathArg);
			}
		}
		settings.inputFilePaths.shrink_to_fit();
		
		if (settings.inputFilePaths.empty())
		{
			loggingManager.logProgramArgMsg(ProgramArgLogs::InFilepathMissing_err);
		}

		// mode
		if (auto OptionalValue = getKargValue("-mode"))
		{
			auto it = svToModeTable.find(*OptionalValue);
			if (it != svToModeTable.end())
			{
				settings.mode = it->second;
			}
			else
			{
				loggingManager.logProgramArgMsg(ProgramArgLogs::ModeInvalid_err);
			}
		}
		else
		{
			loggingManager.logProgramArgMsg(ProgramArgLogs::ModeMissing_err);
		}

		// grouping
		if (auto OptionalValue = getKargValue("-group"))
		{
			auto it = svToGroupingTable.find(*OptionalValue);
			if (it != svToGroupingTable.end())
			{
				settings.grouping = it->second;
			}
			else
			{
				loggingManager.logProgramArgMsg(ProgramArgLogs::GroupInvalid_err);
			}
		}
		else
		{
			loggingManager.logProgramArgMsg(ProgramArgLogs::GroupMissing_err);
		}

		// ---- optional ----
		// mode dependent
		switch (settings.mode)
		{
			case ProcessingMode::Overview: break;
			case ProcessingMode::Validate: ProcessValidationSettings(settings); break;
			case ProcessingMode::Budget: ProcessBudgetSettings(settings); break;
		}

		// output
		settings.logFilePath = getLogPath(false);

		if (auto OptionalValue = getKargValue("-csv"))
		{
			fs::path csvPath = *OptionalValue;
			if (!csvPath.has_extension())
			{
				csvPath += fs::path(".csv");
			}

			if (fs::exists(csvPath))
			{
				loggingManager.logIoMsg(IoLogs::CsvFileAlreadyExists_warn, csvPath);
			}

			settings.csvFilePath = csvPath;
		}

		// TODO: log final settings

		return settings;
	}
};