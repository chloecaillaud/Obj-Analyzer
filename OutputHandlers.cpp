#include "OutputHandlers.h"

ResultOutputterBase::ResultOutputterBase(const ProcessingSettings& settings, LogManager& loggingManager) : settings(settings), loggingManager(loggingManager) {};

// --------------------------------
ResultOutputterOverview::ResultOutputterOverview(const ProcessingSettings& settings, LogManager& loggingManager) : ResultOutputterBase(settings, loggingManager) {};

const std::string ResultOutputterOverview::generateTxtFormattedReport(const PrimDataCollection& asset) const
{
	// grouping dependent: name & group count
	std::string nameCategory;
	std::string groupCategory;
	switch (settings.grouping)
	{
	case DataCollectionGrouping::File:
		nameCategory = "File";
		groupCategory = "Objects:";
		break;
	case DataCollectionGrouping::Object:
		nameCategory = "Object";
		groupCategory = "Groups:";
		break;
	case DataCollectionGrouping::Vertexgroup:
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
};

void ResultOutputterOverview::outputReports(const PrimDataCollection& asset)
{
	// output based no settings
	const auto& txtReport = generateTxtFormattedReport(asset);
	if (!settings.csvFilePath.empty())
	{
		outputToCsv(asset, settings.csvFilePath);
	}
	outputToLog(txtReport);
};

void ResultOutputterOverview::outputToLog(const std::string& txtReport)
{
	loggingManager.log(std::pair(logVerbosity::None, txtReport));
};

void ResultOutputterOverview::outputToLog(const PrimDataCollection& asset)
{
	loggingManager.log(std::pair(logVerbosity::None, generateTxtFormattedReport(asset)));
};

void ResultOutputterOverview::outputToCsv(const PrimDataCollection& asset, const std::filesystem::path& filepath, const char sep)
{
	std::ofstream csvFile(filepath, std::ios::app);
	// TODO: add header if not present
	csvFile << asset.name << sep
		<< asset.vertCount << sep
		<< asset.faceTotalCount << sep
		<< asset.faceTriCount << sep
		<< asset.faceQuadCount << sep
		<< asset.faceNgonCount << sep
		<< asset.pointCount << sep
		<< asset.lineCount << sep
		<< asset.getMaterialCount() << sep
		<< asset.subgroupCount << sep
		<< bool(asset.hasVertNormals) << sep
		<< bool(asset.hasVertColor) << sep
		<< bool(asset.hasUvs) << std::endl
		;
	csvFile.close();

	if (!csvFile)
	{
		loggingManager.logMsgIo(LogPresetIo::CsvFileWriteFail_err, filepath);
	}
};

// --------------------------------
ResultOutputterValidate::ResultOutputterValidate(const ProcessingSettings& settings, LogManager& loggingManager) : ResultOutputterBase(settings, loggingManager) {};

const std::string ResultOutputterValidate::makeLineFromBoolElem(const ValidationBoolElem& validationElem, const bool statValue, const std::string& statName) const
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
};

const std::string ResultOutputterValidate::generateTxtFormattedReport(const PrimDataCollection& asset) const
{
	// grouping dependent: name & group count
	std::string nameCategory;
	switch (settings.grouping)
	{
	case DataCollectionGrouping::File:
		nameCategory = "File";
		break;
	case DataCollectionGrouping::Object:
		nameCategory = "Object";
		break;
	case DataCollectionGrouping::Vertexgroup:
		nameCategory = "Group";
		break;
	};

	std::string textReport = "------------------------------\n";
	textReport += nameCategory + " name: " + asset.name + '\n';
	textReport += "------------------------------\n";
	textReport += makeLineFromBoolElem(settings.validations.containsVerts, asset.vertCount, "Has vertices:");
	textReport += makeLineFromBoolElem(settings.validations.containsFaces, asset.faceTotalCount, "Has faces:");
	textReport += makeLineFromBoolElem(settings.validations.containsTris, asset.faceTriCount, "Has tris:");
	textReport += makeLineFromBoolElem(settings.validations.containsQuads, asset.faceQuadCount, "Has quads:");
	textReport += makeLineFromBoolElem(settings.validations.containsNgons, asset.faceNgonCount, "Has Ngons:");
	textReport += makeLineFromBoolElem(settings.validations.containsLoosePoints, asset.pointCount, "Has loose points:");
	textReport += makeLineFromBoolElem(settings.validations.containsLooseEdges, asset.lineCount, "Has loose edges:");
	textReport += makeLineFromBoolElem(settings.validations.containsMaterials, asset.getMaterialCount(), "Has materials:");
	textReport += makeLineFromBoolElem(settings.validations.containsVertexNormals, asset.hasVertNormals, "Has vertex normals:");
	textReport += makeLineFromBoolElem(settings.validations.containsVertexColor, asset.hasVertColor, "Has vertex colors:");
	textReport += makeLineFromBoolElem(settings.validations.containsUvs, asset.hasUvs, "Has UVs:");
	textReport += makeLineFromBoolElem(settings.validations.MissingName, asset.name.empty(), "Missing Name:");

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
};

inline const std::string ResultOutputterValidate::makeCsvValueFromValidationElem(const ValidationBoolElem& validationElem, const bool statValue) const
{
	if (validationElem.shouldCheck)
	{
		return statValue ? "true" : "false";
	}
	return {};
};

inline const std::string ResultOutputterValidate::makeCsvValueFromValidationElem(const ValidationStringElem& validationElem, const bool isValid) const
{
	if (validationElem.shouldCheck)
	{
		return isValid ? "true" : "false";
	}
	return {};
};

void ResultOutputterValidate::outputReports(const PrimDataCollection& asset)
{
	// output based no settings
	const auto& txtReport = generateTxtFormattedReport(asset);
	if (!settings.csvFilePath.empty())
	{
		outputToCsv(asset, settings.csvFilePath);
	}
	outputToLog(txtReport);
};

void ResultOutputterValidate::outputToConsole(const std::string& txtReport) // TODO: del this, obs replaced by outputToLog
{
	std::cout << txtReport << std::endl;
};

void ResultOutputterValidate::outputToLog(const std::string& txtReport)
{
	loggingManager.log(std::pair(logVerbosity::None, txtReport));
};


void ResultOutputterValidate::outputToLog(const PrimDataCollection& asset)
{
	loggingManager.log(std::pair(logVerbosity::None, generateTxtFormattedReport(asset)));
};

void ResultOutputterValidate::outputToCsv(const PrimDataCollection& asset, const std::filesystem::path& filepath, const char sep)
{
	std::ofstream csvFile(filepath, std::ios::app);
	// TODO: add header if not present

	csvFile << asset.name << sep
		<< makeCsvValueFromValidationElem(settings.validations.containsVerts, asset.vertCount) << sep
		<< makeCsvValueFromValidationElem(settings.validations.containsFaces, asset.faceTotalCount) << sep
		<< makeCsvValueFromValidationElem(settings.validations.containsTris, asset.faceTriCount) << sep
		<< makeCsvValueFromValidationElem(settings.validations.containsQuads, asset.faceQuadCount) << sep
		<< makeCsvValueFromValidationElem(settings.validations.containsNgons, asset.faceNgonCount) << sep
		<< makeCsvValueFromValidationElem(settings.validations.containsLoosePoints, asset.pointCount) << sep
		<< makeCsvValueFromValidationElem(settings.validations.containsLooseEdges, asset.lineCount) << sep
		<< makeCsvValueFromValidationElem(settings.validations.containsMaterials, asset.getMaterialCount()) << sep
		<< makeCsvValueFromValidationElem(settings.validations.containsVertexNormals, asset.hasVertNormals) << sep
		<< makeCsvValueFromValidationElem(settings.validations.containsVertexColor, asset.hasVertColor) << sep
		<< makeCsvValueFromValidationElem(settings.validations.containsUvs, asset.hasUvs) << sep
		<< makeCsvValueFromValidationElem(settings.validations.MissingName, asset.name.empty()) << sep
		<< makeCsvValueFromValidationElem(settings.validations.namePrefix, asset.name.starts_with(settings.validations.namePrefix.substring)) << sep
		<< makeCsvValueFromValidationElem(settings.validations.nameSuffix, asset.name.ends_with(settings.validations.nameSuffix.substring)) << std::endl
		;
	csvFile.close();

	if (!csvFile)
	{
		loggingManager.logMsgIo(LogPresetIo::CsvFileWriteFail_err, filepath);
	}
};

// --------------------------------
ResultOutputterBudget::ResultOutputterBudget(const ProcessingSettings& settings, LogManager& loggingManager) : ResultOutputterBase(settings, loggingManager) {};

const std::string ResultOutputterBudget::makeLineFromBudgetElem(const BudgetUint32Elem& budgetElem, const uint32_t statValue, const std::string& statName) const
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
};

const std::string ResultOutputterBudget::generateTxtFormattedReport(const PrimDataCollection& asset) const
{
	// grouping dependent: name & group count
	std::string nameCategory;
	std::string groupCategory;
	switch (settings.grouping)
	{
	case DataCollectionGrouping::File:
		nameCategory = "File";
		groupCategory = "Object";
		break;
	case DataCollectionGrouping::Object:
		nameCategory = "Object";
		groupCategory = "Group";
		break;
	case DataCollectionGrouping::Vertexgroup:
		nameCategory = "Group";
		groupCategory = "N/A";
		break;
	};

	std::string textReport = "------------------------------\n";
	textReport += nameCategory + " name: " + asset.name + '\n';
	textReport += "------------------------------\n";
	textReport += makeLineFromBudgetElem(settings.budgets.verts, asset.vertCount, "Vertex count:");
	textReport += "Face count:\n";
	textReport += makeLineFromBudgetElem(settings.budgets.faceTotals, asset.faceTriCount, "  Total:");
	textReport += makeLineFromBudgetElem(settings.budgets.faceTris, asset.faceTriCount, "  tri:");
	textReport += makeLineFromBudgetElem(settings.budgets.faceQuads, asset.faceQuadCount, "  Quad:");
	textReport += makeLineFromBudgetElem(settings.budgets.faceNgons, asset.faceNgonCount, "  Ngon:");
	textReport += makeLineFromBudgetElem(settings.budgets.points, asset.pointCount, "Loose point count:");
	textReport += makeLineFromBudgetElem(settings.budgets.lines, asset.lineCount, "Loose edge count:");
	textReport += makeLineFromBudgetElem(settings.budgets.materials, uint32_t(asset.getMaterialCount()), "Material count:");
	textReport += makeLineFromBudgetElem(settings.budgets.groups, asset.subgroupCount, format("{} count:", groupCategory));

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
};

const std::string ResultOutputterBudget::makeCsvValueFromBudgetElem(const BudgetUint32Elem& budgetElem, const uint32_t statValue) const
{
	if (budgetElem.shouldCheck)
	{
		if (budgetElem.value != 0)
		{
			return std::to_string((double(statValue) / budgetElem.value) * 100); //convert to percentage
		}
		else
		{
			return statValue == 0 ? "PASS" : "FAIL";
		}
	}
	return {};
};

void ResultOutputterBudget::outputReports(const PrimDataCollection& asset)
{
	// output based no settings
	const auto& txtReport = generateTxtFormattedReport(asset);
	if (!settings.csvFilePath.empty())
	{
		outputToCsv(asset, settings.csvFilePath);
	}
	outputToLog(txtReport);
};

void ResultOutputterBudget::outputToConsole(const std::string& txtReport) // TODO: del this, obs replaced by outputToLog
{
	std::cout << txtReport << std::endl;
}

void ResultOutputterBudget::outputToLog(const std::string& txtReport)
{
	loggingManager.log(std::pair(logVerbosity::None, txtReport));
};

void ResultOutputterBudget::outputToLog(const PrimDataCollection& asset)
{
	loggingManager.log(std::pair(logVerbosity::None, generateTxtFormattedReport(asset)));
};

void ResultOutputterBudget::outputToCsv(const PrimDataCollection& asset, const std::filesystem::path& filepath, const char sep)
{
	std::ofstream csvFile(filepath, std::ios::app);
	// TODO: add header if not present
	csvFile << asset.name << sep
		<< makeCsvValueFromBudgetElem(settings.budgets.verts, asset.vertCount) << sep
		<< makeCsvValueFromBudgetElem(settings.budgets.faceTotals, asset.faceTotalCount) << sep
		<< makeCsvValueFromBudgetElem(settings.budgets.faceTris, asset.faceTriCount) << sep
		<< makeCsvValueFromBudgetElem(settings.budgets.faceQuads, asset.faceQuadCount) << sep
		<< makeCsvValueFromBudgetElem(settings.budgets.faceNgons, asset.faceNgonCount) << sep
		<< makeCsvValueFromBudgetElem(settings.budgets.points, asset.pointCount) << sep
		<< makeCsvValueFromBudgetElem(settings.budgets.lines, asset.lineCount) << sep
		<< makeCsvValueFromBudgetElem(settings.budgets.materials, uint32_t(asset.getMaterialCount())) << sep
		<< makeCsvValueFromBudgetElem(settings.budgets.groups, asset.subgroupCount) << std::endl
		;
	csvFile.close();

	if (!csvFile)
	{
		loggingManager.logMsgIo(LogPresetIo::CsvFileWriteFail_err, filepath);
	}
};