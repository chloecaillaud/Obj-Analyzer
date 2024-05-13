#pragma once
#include <filesystem>
#include <fstream>
#include <vector>
#include <format>
#include <iostream>

#include "Settings.cpp"
#include "LogManager.cpp"

using namespace std;
namespace fs = std::filesystem;

class ResultOutputterBase
{
protected:
	const ProcessingSettings& settings;
	LogManager& loggingManager;
public:
	ResultOutputterBase(const ProcessingSettings& settings, LogManager& loggingManager) : settings(settings), loggingManager(loggingManager) {}

	virtual void outputReports(const AssetData&) { cout << "not implement" << endl; }
	virtual void outputToLog(const AssetData&) { cout << "not implement" << endl; }
	virtual void outputToCsv(const AssetData&, const fs::path&, const char = ',') { cout << "not implement" << endl; }
};

class ResultOutputterOverview : public ResultOutputterBase
{
	static constexpr const char* reportTxtTempate =
		"------------------------------\n"
		"{0} name: {1}\n"
		"------------------------------\n"
		"  Vertex count:       {4:L}\n"
		"  Face count:\n"
		"    Total:            {5:L}\n"
		"    Tri:              {6:L}\n"
		"    Quad:             {7:L}\n"
		"    Ngon:             {8:L}\n"
		"  Loose point count:  {9:L}\n"
		"  Loose edge count:   {10:L}\n"
		"  Material count:     {11:L}\n"
		"  {2:<20}{3:L}\n"
		"  Has vertex normals: {12}\n"
		"  Has vertex color:   {13}\n"
		"  Has UVs             {14}\n"
		"------------------------------\n"
	;

	const string generateTxtFormattedReport(const AssetData& asset) const
	{
		// grouping dependent: name & group count
		string nameCategory;
		string groupCategory;
		switch (settings.grouping)
		{
		case assetDataGrouping::File:
			nameCategory = "File";
			groupCategory = "Objects:";
			break;
		case assetDataGrouping::Object:
			nameCategory = "Object";
			groupCategory = "Groups:";
			break;
		case assetDataGrouping::Vertexgroup:
			nameCategory = "Group";
			groupCategory = "N/A";
			break;
		};

		return format(reportTxtTempate,
			nameCategory,             //  0
			asset.name,               //  1
			groupCategory,            //  2
			asset.subgroupCount,      //  3
			asset.vertCount,          //  4
			asset.faceTotalCount,     //  5
			asset.faceTriCount,       //  6
			asset.faceQuadCount,      //  7
			asset.faceNgonCount,      //  8
			asset.pointCount,         //  9
			asset.lineCount,          // 10
			asset.getMaterialCount(), // 11
			asset.hasVertNormals,     // 12
			asset.hasVertColor,       // 13
			asset.hasUvs              // 14
		);
	}

	void outputToLog(const string& txtReport)
	{
		loggingManager.logMsg(pair(logVerbosity::None, txtReport));
	}

public:
	ResultOutputterOverview(const ProcessingSettings& settings, LogManager& loggingManager) : ResultOutputterBase(settings, loggingManager) {}

	void outputReports(const AssetData& asset) override
	{
		// output based no settings
		const auto& txtReport = generateTxtFormattedReport(asset);
		if (!settings.csvFilePath.empty())
		{
			outputToCsv(asset, settings.csvFilePath);
		}
		outputToLog(txtReport);
	}

	void outputToCsv(const AssetData& asset, const fs::path& filepath, const char sep = ',') override
	{
		ofstream csvFile(filepath, ios::app);
		// TODO: add header if not present
		csvFile << asset.name             << sep
			<< asset.vertCount            << sep
			<< asset.faceTotalCount       << sep
			<< asset.faceTriCount         << sep
			<< asset.faceQuadCount        << sep
			<< asset.faceNgonCount        << sep
			<< asset.pointCount           << sep
			<< asset.lineCount            << sep
			<< asset.getMaterialCount()   << sep
			<< asset.subgroupCount        << sep
			<< bool(asset.hasVertNormals) << sep
			<< bool(asset.hasVertColor)   << sep
			<< bool(asset.hasUvs)         << endl
			;
		csvFile.close();

		if (!csvFile)
		{
			loggingManager.logIoMsg(IoLogs::CsvFileWriteFail_err, filepath);
		}

	}

	void outputToLog(const AssetData& asset) override
	{
		loggingManager.logMsg(pair(logVerbosity::None, generateTxtFormattedReport(asset)));
	}
};

class ResultOutputterValidate : public ResultOutputterBase
{
	const string makeLineFromBoolElem(const ValidationBoolElem& validationElem, const bool statValue, const string& statName) const
	{
		if (validationElem.shouldCheck)
		{
			return format("{:<20} ({})  {}\n",
				statName,
				validationElem.expectedValue ? "T" : "F",
				statValue == validationElem.expectedValue ? "PASS" : "FAIL"
			);
		}
		return {};
	}

	const string generateTxtFormattedReport(const AssetData& asset) const
	{
		// grouping dependent: name & group count
		string nameCategory;
		switch (settings.grouping)
		{
		case assetDataGrouping::File:
			nameCategory = "File";
			break;
		case assetDataGrouping::Object:
			nameCategory = "Object";
			break;
		case assetDataGrouping::Vertexgroup:
			nameCategory = "Group";
			break;
		};

		string textReport = "------------------------------\n";
		textReport += nameCategory + " name: " + asset.name + '\n';
		textReport += "------------------------------\n";
		textReport += makeLineFromBoolElem(settings.validations.containsVerts,         asset.vertCount,          "Has vertices:");
		textReport += makeLineFromBoolElem(settings.validations.containsFaces,         asset.faceTotalCount,     "Has faces:");
		textReport += makeLineFromBoolElem(settings.validations.containsTris,          asset.faceTriCount,       "Has tris:");
		textReport += makeLineFromBoolElem(settings.validations.containsQuads,         asset.faceQuadCount,      "Has quads:");
		textReport += makeLineFromBoolElem(settings.validations.containsNgons,         asset.faceNgonCount,      "Has Ngons:");
		textReport += makeLineFromBoolElem(settings.validations.containsLoosePoints,   asset.pointCount,         "Has loose points:");
		textReport += makeLineFromBoolElem(settings.validations.containsLooseEdges,    asset.lineCount,          "Has loose edges:");
		textReport += makeLineFromBoolElem(settings.validations.containsMaterials,     asset.getMaterialCount(), "Has materials:");
		textReport += makeLineFromBoolElem(settings.validations.containsVertexNormals, asset.hasVertNormals,     "Has vertex normals:");
		textReport += makeLineFromBoolElem(settings.validations.containsVertexColor,   asset.hasVertColor,       "Has vertex colors:");
		textReport += makeLineFromBoolElem(settings.validations.containsUvs,           asset.hasUvs,             "Has UVs:");
		textReport += makeLineFromBoolElem(settings.validations.MissingName,           asset.name.empty(),       "Missing Name:");

		if (settings.validations.namePrefix.shouldCheck)
		{
			textReport += "Name prefix:              ";
			textReport += asset.name.starts_with(settings.validations.namePrefix.substring) ? "PASS\n" : "FAIL\n";
		}
		if (settings.validations.nameSuffix.shouldCheck)
		{
			textReport += "Name suffix:              ";
			textReport += asset.name.ends_with(settings.validations.nameSuffix.substring) ? "PASS\n" : "FAIL\n";
		}
		textReport += "------------------------------\n";
		if (textReport.find("FAIL") != textReport.npos)
		{
			textReport += "Status:  FAIL\n";
		}
		else
		{
			textReport += "Status:  PASS\n";
		}
		textReport += "------------------------------\n";

		// TODO: PASS/FAIL color
		return textReport;
	}

	inline const string makeCsvValueFromValidationElem(const ValidationBoolElem& validationElem, const bool statValue) const
	{
		if (validationElem.shouldCheck)
		{
			return statValue ? "true" : "false";
		}
		return {};
	}

	inline const string makeCsvValueFromValidationElem(const ValidationStringElem& validationElem, const bool isValid) const
	{
		if (validationElem.shouldCheck)
		{
			return isValid ? "true" : "false";
		}
		return {};
	}

	void outputToConsole(const string& txtReport)
	{
		cout << txtReport << endl;
	}

	void outputToLog(const string & txtReport)
	{
		loggingManager.logMsg(pair(logVerbosity::None, txtReport));
	}


public:
	ResultOutputterValidate(const ProcessingSettings& settings, LogManager& loggingManager) : ResultOutputterBase(settings, loggingManager) {}

	void outputReports(const AssetData& asset) override
	{
		// output based no settings
		const auto& txtReport = generateTxtFormattedReport(asset);
		if (!settings.csvFilePath.empty())
		{
			outputToCsv(asset, settings.csvFilePath);
		}
		outputToLog(txtReport);
	}

	void outputToCsv(const AssetData& asset, const fs::path& filepath, const char sep = ',') override
	{
		ofstream csvFile(filepath, ios::app);
		// TODO: add header if not present
		
		csvFile << asset.name << sep
			<< makeCsvValueFromValidationElem(settings.validations.containsVerts,         asset.vertCount)          << sep
			<< makeCsvValueFromValidationElem(settings.validations.containsFaces,         asset.faceTotalCount)     << sep
			<< makeCsvValueFromValidationElem(settings.validations.containsTris,          asset.faceTriCount)       << sep
			<< makeCsvValueFromValidationElem(settings.validations.containsQuads,         asset.faceQuadCount)      << sep
			<< makeCsvValueFromValidationElem(settings.validations.containsNgons,         asset.faceNgonCount)      << sep
			<< makeCsvValueFromValidationElem(settings.validations.containsLoosePoints,   asset.pointCount)         << sep
			<< makeCsvValueFromValidationElem(settings.validations.containsLooseEdges,    asset.lineCount)          << sep
			<< makeCsvValueFromValidationElem(settings.validations.containsMaterials,     asset.getMaterialCount()) << sep
			<< makeCsvValueFromValidationElem(settings.validations.containsVertexNormals, asset.hasVertNormals)     << sep
			<< makeCsvValueFromValidationElem(settings.validations.containsVertexColor,   asset.hasVertColor)       << sep
			<< makeCsvValueFromValidationElem(settings.validations.containsUvs,           asset.hasUvs)             << sep
			<< makeCsvValueFromValidationElem(settings.validations.MissingName,           asset.name.empty())       << sep
			<< makeCsvValueFromValidationElem(settings.validations.namePrefix,            asset.name.starts_with(settings.validations.namePrefix.substring)) << sep
			<< makeCsvValueFromValidationElem(settings.validations.nameSuffix,            asset.name.ends_with(  settings.validations.nameSuffix.substring)) << endl
			;
		csvFile.close();

		if (!csvFile)
		{
			loggingManager.logIoMsg(IoLogs::CsvFileWriteFail_err, filepath);
		}
	}

	void outputToLog(const AssetData& asset) override
	{
		loggingManager.logMsg(pair(logVerbosity::None, generateTxtFormattedReport(asset)));
	}
};

class ResultOutputterBudget : public ResultOutputterBase
{
	const string makeLineFromBudgetElem(const BudgetUint32Elem& budgetElem, const uint32_t statValue, const string& statName) const
	{
		double budgetRatio = double(statValue) / budgetElem.value;

		if (budgetElem.shouldCheck)
		{
			return format("{:<18} {} {:12.2f} ({}/{})\n",
				statName,
				budgetElem.value >= statValue ? "PASS" : "FAIL",
				budgetElem.value == 0 ? -1.0 : budgetRatio * 100,
				statValue,
				budgetElem.value
			);
		}
		return {};
	}

	const string generateTxtFormattedReport(const AssetData& asset) const
	{
		// grouping dependent: name & group count
		string nameCategory;
		string groupCategory;
		switch (settings.grouping)
		{
		case assetDataGrouping::File:
			nameCategory = "File";
			groupCategory = "Object";
			break;
		case assetDataGrouping::Object:
			nameCategory = "Object";
			groupCategory = "Group";
			break;
		case assetDataGrouping::Vertexgroup:
			nameCategory = "Group";
			groupCategory = "N/A";
			break;
		};

		string textReport = "------------------------------\n";
		textReport += nameCategory + " name: " + asset.name + '\n';
		textReport += "------------------------------\n";
		textReport += makeLineFromBudgetElem(settings.budgets.verts,      asset.vertCount,                    "Vertex count:");
		textReport += "Face count:\n";
		textReport += makeLineFromBudgetElem(settings.budgets.faceTotals, asset.faceTriCount,                 "  Total:");
		textReport += makeLineFromBudgetElem(settings.budgets.faceTris,   asset.faceTriCount,                 "  tri:");
		textReport += makeLineFromBudgetElem(settings.budgets.faceQuads,  asset.faceQuadCount,                "  Quad:");
		textReport += makeLineFromBudgetElem(settings.budgets.faceNgons,  asset.faceNgonCount,                "  Ngon:");
		textReport += makeLineFromBudgetElem(settings.budgets.points,     asset.pointCount,                   "Loose point count:");
		textReport += makeLineFromBudgetElem(settings.budgets.lines,      asset.lineCount,                    "Loose edge count:");
		textReport += makeLineFromBudgetElem(settings.budgets.materials,  uint32_t(asset.getMaterialCount()), "Material count:");
		textReport += makeLineFromBudgetElem(settings.budgets.groups,     asset.subgroupCount,                format("{} count:", groupCategory));

		if (settings.validations.namePrefix.shouldCheck)
		{
			textReport += "Name prefix:             ";
			textReport += asset.name.starts_with(settings.validations.namePrefix.substring) ? "PASS\n" : "FAIL\n";
		}
		if (settings.validations.nameSuffix.shouldCheck)
		{
			textReport += "Name suffix:             ";
			textReport += asset.name.ends_with(settings.validations.nameSuffix.substring) ? "PASS\n" : "FAIL\n";
		}
		textReport += "------------------------------\n";
		if (textReport.find("FAIL") != textReport.npos)
		{
			textReport += "Status:  FAIL\n";
		}
		else
		{
			textReport += "Status:  PASS\n";
		}
		textReport += "------------------------------\n";

		// TODO: PASS/FAIL color
		return textReport;
	}

	const string makeCsvValueFromBudgetElem(const BudgetUint32Elem& budgetElem, const uint32_t statValue) const
	{
		if (budgetElem.shouldCheck)
		{
			if (budgetElem.value != 0)
			{
				return to_string((double(statValue) / budgetElem.value) * 100); //convert to percentage
			}
			else
			{
				return statValue == 0 ? "PASS" : "FAIL";
			}
		}
		return {};
	}

	void outputToConsole(const string& txtReport)
	{
		cout << txtReport << endl;
	}

	void outputToLog(const string& txtReport)
	{
		loggingManager.logMsg(pair(logVerbosity::None, txtReport));
	}

public:
	ResultOutputterBudget(const ProcessingSettings& settings, LogManager& loggingManager) : ResultOutputterBase(settings, loggingManager) {}

	void outputReports(const AssetData& asset) override
	{
		// output based no settings
		const auto& txtReport = generateTxtFormattedReport(asset);
		if (!settings.csvFilePath.empty())
		{
			outputToCsv(asset, settings.csvFilePath);
		}
		outputToLog(txtReport);
	}

	void outputToCsv(const AssetData& asset, const fs::path& filepath, const char sep = ',') override
	{
		ofstream csvFile(filepath, ios::app);
		// TODO: add header if not present
		csvFile << asset.name << sep
			<< makeCsvValueFromBudgetElem(settings.budgets.verts,      asset.vertCount)                    << sep
			<< makeCsvValueFromBudgetElem(settings.budgets.faceTotals, asset.faceTotalCount)               << sep
			<< makeCsvValueFromBudgetElem(settings.budgets.faceTris,   asset.faceTriCount)                 << sep
			<< makeCsvValueFromBudgetElem(settings.budgets.faceQuads,  asset.faceQuadCount)                << sep
			<< makeCsvValueFromBudgetElem(settings.budgets.faceNgons,  asset.faceNgonCount)                << sep
			<< makeCsvValueFromBudgetElem(settings.budgets.points,     asset.pointCount)                   << sep
			<< makeCsvValueFromBudgetElem(settings.budgets.lines,      asset.lineCount)                    << sep
			<< makeCsvValueFromBudgetElem(settings.budgets.materials,  uint32_t(asset.getMaterialCount())) << sep
			<< makeCsvValueFromBudgetElem(settings.budgets.groups,     asset.subgroupCount)                << endl
			;
		csvFile.close();

		if (!csvFile)
		{
			loggingManager.logIoMsg(IoLogs::CsvFileWriteFail_err, filepath);
		}
	}

	void outputToLog(const AssetData& asset) override
	{
		loggingManager.logMsg(pair(logVerbosity::None, generateTxtFormattedReport(asset)));
	}
};