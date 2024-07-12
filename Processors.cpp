#include "Processors.h"
#include "DataCollection.h"

std::string_view LineProcessor::getLineType() const
{
	return std::string_view(lineStr.data(), lineStr.find(' '));
}

uint32_t LineProcessor::getValueCount() const
{
	uint32_t valueCount = 0;

	for (char character : lineStr)
	{
		// all values are prefixed with a space
		// therfore space count = values count (unless malformed file)
		if (character == ' ') { valueCount++; }
	}
	return valueCount;
}

std::vector<std::string_view> LineProcessor::getValues() const
{
	std::vector<std::string_view> values;

	size_t lastDelimiter = 0;
	for (size_t i = 0; i < lineStr.size(); i++)
	{
		if (lineStr[i] == ' ')
		{
			// skip first delim since represent line type not value
			if (lastDelimiter != 0)
			{
				values.push_back(std::string_view(
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
		values.push_back(std::string_view(
			lineStr.data() + lastDelimiter,
			lineStr.data() + lineStr.size()
		));
	}
	return values;
}


// --------------------------------
FileProcessor::FileProcessor(const std::filesystem::path& filepath, const ProcessingSettings& settings, LogManager& loggingManager)
	: filepath(filepath)
	, settings(settings)
	, loggingManager(loggingManager)
	, PrimDataCollections({ PrimDataCollection(filepath.stem().string()) }) //default obj for malformed file lacking "g" or "o" lines or when settings mode:File
{};

const std::filesystem::path& FileProcessor::getFilepath() const
{
	return filepath;
}

PrimDataCollection& FileProcessor::getCurrentObject()
{
	return PrimDataCollections.back();
}

void FileProcessor::processFile()
{
	loggingManager.logMsgProcessing(LogPresetProcessing::ProcessingStart_log);
	loggingManager.logMsgIo(LogPresetIo::InPathRead_log, filepath);
	auto start_time = std::chrono::system_clock::now(); // TODO: move this?

	std::ifstream file(filepath, std::ios::binary);
	LineProcessor lineProcessor;

	lineNum = 0;
	// itt over lines in file
	while (getline(file, lineProcessor.lineStr))
	{
		lineNum++;
		const std::string_view lineType = lineProcessor.getLineType();
		// prim lineType
		if (lineType == "v" && settings.areVertsRelevent())
		{
			PrimDataCollection& currObj = getCurrentObject();
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
			if (settings.grouping == DataCollectionGrouping::Object)
			{
				// treat as asset grouping
				if (!lineValues.empty())
				{
					PrimDataCollections.push_back(PrimDataCollection{ lineValues.front() });
					if (lineValues.size() > 1)
					{
						loggingManager.logMsgProcessing(LogPresetProcessing::ONameMoreThanOne_warn, lineNum);
					}
				}
				else
				{
					loggingManager.logMsgProcessing(LogPresetProcessing::ONameMissing_warn, lineNum);
				}
			}
			else if (settings.grouping == DataCollectionGrouping::File && settings.areSubGroupsRelevent())
			{
				// treat as sub grouping
				getCurrentObject().subgroupCount++;
			}
		}
		else if (lineType == "g")
		{
			const auto& lineValues = lineProcessor.getValues();
			if (settings.grouping == DataCollectionGrouping::Vertexgroup)
			{
				// treat as asset grouping
				if (!lineValues.empty() && lineValues.front() != "off")
				{
					PrimDataCollections.push_back(PrimDataCollection{ lineValues.front() });
					if (lineValues.size() > 1)
					{
						loggingManager.logMsgProcessing(LogPresetProcessing::GNameMoreThanOne_warn, lineNum);
					}
				}
				else if (lineValues.empty())
				{
					loggingManager.logMsgProcessing(LogPresetProcessing::GNameMissing_warn, lineNum);
				}
			}
			else if (settings.grouping == DataCollectionGrouping::Object
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
		if (settings.isMultiFile())
		{
			loggingManager.logMsgIo(LogPresetIo::InFileReadFail_warn, filepath);
		}
		else
		{
			loggingManager.logMsgIo(LogPresetIo::InFileReadFail_err, filepath);
		}
	}
	// log elapsed time
	loggingManager.logMsgProcessing(LogPresetProcessing::ProcessingEndStats_log,
		std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time),
		lineNum
		);
	loggingManager.logMsgProcessing(LogPresetProcessing::ProcessingEnd_log);
}

// --------------------------------
bool ProgramArgParcer::convertSvToBool(std::string_view sv, bool defaultValue) const
{
	auto it = svToBoolTable.find(sv);
	if (it != svToBoolTable.end())
	{
		return it->second;
	}
	else if (!sv.empty())
	{
		loggingManager.logMsgProgramArg(LogPresetProgramArg::boolValueInvalid_warn, sv);
	}
	return defaultValue;
}

std::optional<std::string_view> ProgramArgParcer::getKargValue(std::string_view key) const
{
	auto it = keywordArgs.find(key);
	if (it != keywordArgs.end())
	{
		return it->second;
	}
	return {};
}

void ProgramArgParcer::ProcessValidationSettings(ProcessingSettings& settings)
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
		settings.validations.namePrefix.substring = std::string(*OptionalValue);
	}

	if (auto OptionalValue = getKargValue("-suffix"))
	{
		settings.validations.nameSuffix.shouldCheck = true;
		settings.validations.nameSuffix.substring = std::string(*OptionalValue);
	}
}

void ProgramArgParcer::setValidationBoolElem(ValidationBoolElem& elem, std::string_view key, bool defaultExpectedValue)
{
	if (auto OptionalValue = getKargValue(key))
	{
		elem.shouldCheck = true;
		elem.expectedValue = convertSvToBool(*OptionalValue, defaultExpectedValue);
	}
}

void ProgramArgParcer::ProcessBudgetSettings(ProcessingSettings& settings)
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

void ProgramArgParcer::setBudgetUint32Elem(BudgetUint32Elem& elem, const std::string_view key)
{
	if (auto OptionalValue = getKargValue(key))
	{
		elem.shouldCheck = true;

		//store in temp var since elem.value is actully a 31 bit field
		uint32_t resultValue;
		std::string_view& argValue = *OptionalValue;
		auto convertionErrStatus = std::from_chars(argValue.data(), argValue.data() + argValue.size(), resultValue).ec;
		if (convertionErrStatus == std::errc())
		{
			// no issues
			elem.value = resultValue;
		}
		else if (convertionErrStatus == std::errc::invalid_argument)
		{
			loggingManager.logMsgProgramArg(LogPresetProgramArg::BudgetInvalid_err, key);
		}
		else if (convertionErrStatus == std::errc::result_out_of_range)
		{
			loggingManager.logMsgProgramArg(LogPresetProgramArg::BudgetOutOfRange_warn, key, std::to_string(0x7FFFFFFF)); // max val for BudgetUint32Elem (2^31-1)
		}
	}
}

void ProgramArgParcer::getFilesFromDir(const std::filesystem::path& dirPath, std::vector<std::filesystem::path>& filepathCollection)
{
	for (auto const& entry : std::filesystem::directory_iterator(dirPath, std::filesystem::directory_options::skip_permission_denied))
	{
		if (std::filesystem::is_regular_file(entry)
			&& entry.path().extension() == ".obj"
			)
		{
			filepathCollection.push_back(entry.path());
		}
	}
}


ProgramArgParcer::ProgramArgParcer(const int& argc,char* argv[], LogManager& loggingManager)
	: loggingManager(loggingManager)
{
	std::string_view currentArgKey;
	for (int i = 1; i < argc; i++)
	{
		// 1 <= i <= argc-1 since 0 is prog name and argc is null term.
		if (argv[i][0] == '-')
		{
			// is keyword arg key
			currentArgKey = std::string_view(argv[i]);
			keywordArgs[currentArgKey];
		}
		else if (!currentArgKey.empty())
		{
			// keyword arg value
			keywordArgs[currentArgKey] = std::string_view(argv[i]);
		}
		else
		{
			// positional arg
			positionalArgs.push_back(std::string_view(argv[i]));
		}
	}
}

bool ProgramArgParcer::hasHelpArg() const
{
	return getKargValue("-h").has_value();
}

std::filesystem::path ProgramArgParcer::getLogPath(bool logWarnings) const
{
	if (auto OptionalValue = getKargValue("-log"))
	{
		std::filesystem::path logPath = *OptionalValue;
		if (!logPath.has_extension())
		{
			logPath += std::filesystem::path(".log");
		}

		if (logWarnings && std::filesystem::exists(logPath))
		{
			loggingManager.logMsgIo(LogPresetIo::LogFileAlreadyExists_warn, logPath);
		}

		return logPath;
	}
	return std::filesystem::path();
}

ProcessingSettings ProgramArgParcer::asSettings()
{
	ProcessingSettings settings;

	// ---- req ----
	// obj file paths
	bool isMultiFile = positionalArgs.size() == 1;
	settings.inputFilePaths.reserve(positionalArgs.size());
	for (std::filesystem::path pathArg : positionalArgs)
	{
		if (std::filesystem::exists(pathArg))
		{
			if (std::filesystem::is_regular_file(pathArg))
			{
				if (pathArg.extension() == ".obj")
				{
					settings.inputFilePaths.push_back(std::move(std::filesystem::path(pathArg)));
				}
				else
				{
					loggingManager.logMsgIo(isMultiFile ? LogPresetIo::InFileInvalidType_warn : LogPresetIo::InFileInvalidType_err, pathArg);
				}
			}
			else if (std::filesystem::is_directory(pathArg))
			{
				getFilesFromDir(pathArg, settings.inputFilePaths);
			}
		}
		else
		{
			loggingManager.logMsgIo(isMultiFile ? LogPresetIo::InPathInvalid_warn : LogPresetIo::InPathInvalid_err, pathArg);
		}
	}
	settings.inputFilePaths.shrink_to_fit();
		
	if (settings.inputFilePaths.empty())
	{
		loggingManager.logMsgProgramArg(LogPresetProgramArg::InFilepathMissing_err);
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
			loggingManager.logMsgProgramArg(LogPresetProgramArg::ModeInvalid_err);
		}
	}
	else
	{
		loggingManager.logMsgProgramArg(LogPresetProgramArg::ModeMissing_err);
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
			loggingManager.logMsgProgramArg(LogPresetProgramArg::GroupInvalid_err);
		}
	}
	else
	{
		loggingManager.logMsgProgramArg(LogPresetProgramArg::GroupMissing_err);
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
		std::filesystem::path csvPath = *OptionalValue;
		if (!csvPath.has_extension())
		{
			csvPath += std::filesystem::path(".csv");
		}

		if (std::filesystem::exists(csvPath))
		{
			loggingManager.logMsgIo(LogPresetIo::CsvFileAlreadyExists_warn, csvPath);
		}

		settings.csvFilePath = csvPath;
	}

	// TODO: log final settings

	return settings;
}