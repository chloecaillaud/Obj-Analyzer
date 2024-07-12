#pragma once

#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>
#include <map>
#include <optional>
#include <chrono>

#include "Settings.h"
#include "LogManager.h"


class LineProcessor
{
public:
	std::string lineStr;

	//std::string_view getLineType() const; // TODO: test if outputting enum would be faster
	std::string_view getLineType() const;

	uint32_t getValueCount() const;
	std::vector<std::string_view> getValues() const;
};

class FileProcessor
{
protected:
	const std::filesystem::path& filepath;
	LogManager& loggingManager;
	uint64_t lineNum = 0;

public:
	const ProcessingSettings& settings;
	std::vector<PrimDataCollection> PrimDataCollections;

	FileProcessor(const std::filesystem::path& filepath, const ProcessingSettings& settings, LogManager& loggingManager);

	const std::filesystem::path& getFilepath() const;
	PrimDataCollection& getCurrentObject();

	void processFile();
};

class ProgramArgParcer
{
	LogManager& loggingManager;

	const std::map<const std::string_view, bool>& svToBoolTable
	{
		{ "true" , true },
		{ "t"    , true },
		{ "1"    , true },
		{ "false", false },
		{ "f"    , false },
		{ "0"    , false },
	};

	const std::map<std::string_view, ProcessingMode> svToModeTable
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

	const std::map<std::string_view, DataCollectionGrouping> svToGroupingTable
	{
		{ "f"     , DataCollectionGrouping::File },
		{ "file"  , DataCollectionGrouping::File },
		{ "o"     , DataCollectionGrouping::Object },
		{ "object", DataCollectionGrouping::Object },
		{ "g"     , DataCollectionGrouping::Vertexgroup },
		{ "group" , DataCollectionGrouping::Vertexgroup },
	};

	std::map<std::string_view, std::string_view> keywordArgs;
	std::vector<std::string_view> positionalArgs;

	bool convertSvToBool(std::string_view sv, bool defaultValue) const;

	std::optional<std::string_view> getKargValue(std::string_view key) const;

	void ProcessValidationSettings(ProcessingSettings& settings);
	void ProcessBudgetSettings(ProcessingSettings& settings);

	void setValidationBoolElem(ValidationBoolElem& elem, std::string_view key, bool defaultExpectedValue);
	void setBudgetUint32Elem(BudgetUint32Elem& elem, const std::string_view key);
	
	void getFilesFromDir(const std::filesystem::path& dirPath, std::vector<std::filesystem::path>& filepathCollection);

public:
	ProgramArgParcer(const int& argc, char* argv[], LogManager& loggingManager);

	bool hasHelpArg() const;

	std::filesystem::path getLogPath(bool logWarnings = true) const;

	ProcessingSettings asSettings();
};