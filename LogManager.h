#pragma once

#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <format>
#include <chrono>
#include <exception>

enum class logVerbosity : uint8_t
{
	None,
	Log,
	Warning,
	Error,
};

enum class LogPresetProgramArg : uint8_t
{
	GenericMissing_err,
	GenericMissing_warn,
	GenericInvaid_err,
	GenericInvaid_warn,
	InFilepathMissing_err,
	ModeMissing_err,
	ModeInvalid_err,
	GroupMissing_err,
	GroupInvalid_err,
	BudgetInvalid_err,
	BudgetOutOfRange_warn,
	BudgetOptions_log,
	boolValueInvalid_warn,
	ValidateOptions_log,
};

enum class LogPresetIo : uint8_t
{
	InPathInvalid_err,
	InPathInvalid_warn,
	InFileInvalidType_err,
	InFileInvalidType_warn,
	InFileReadFail_err,
	InFileReadFail_warn,
	InPathRead_log,
	LogFileAlreadyExists_warn,
	LogFileWriteFail_err,
	LogPathWrite_log,
	CsvFileAlreadyExists_warn,
	CsvFileWriteFail_err,
	CsvPathWrite_log,
};

enum class LogPresetProcessing : uint8_t
{
	GenericMissingValues_warn,
	OMalformed_warn,
	ONameMissing_warn,
	ONameMoreThanOne_warn,
	GNameMissing_warn,
	GNameMoreThanOne_warn,
	ProcessingStart_log,
	ProcessingEnd_log,
	ProcessingEndStats_log,
};

class Logger
{
	std::ofstream fileWriteStream;
	bool bWriteToFile = false;
	bool bWriteToConsole = false;

public:
	Logger(bool enableWriteToConsole = true);
	~Logger();

	void open(const std::filesystem::path& filepath, std::ios_base::openmode openmode);
	void close();

	template<typename T>
	Logger& operator<<(const T& data);

	// defined functionaly for custom endl
	typedef Logger& (*LoggerManip)(Logger&);
	Logger& operator<<(LoggerManip manip);

	// custom endl
	static Logger& endl(Logger& logger);
};

class LoggedErrorException
{
	const std::string msg;
public:
	LoggedErrorException(const std::string& msg);

	const char* what() const;
};

class LogManager
{
	// pre-defined messages for specific logs
	const std::map<LogPresetProgramArg, std::pair<logVerbosity, std::string>> logPresetToMsgProgramArg
	{
		{ LogPresetProgramArg::GenericMissing_err,    { logVerbosity::Error   , "Missing argument '{0}'." }},
		{ LogPresetProgramArg::GenericMissing_warn,   { logVerbosity::Warning , "Missing argument '{0}', defaulting to {1}." }},
		{ LogPresetProgramArg::GenericInvaid_err,     { logVerbosity::Error   , "Invalid argument '{0}'" }},
		{ LogPresetProgramArg::GenericInvaid_warn,    { logVerbosity::Warning , "Invalid argument '{0}', defaulting to {1}." }},
		{ LogPresetProgramArg::InFilepathMissing_err, { logVerbosity::Error   , "No valid file(s) to analize specified." }},
		{ LogPresetProgramArg::ModeMissing_err,       { logVerbosity::Error   , "Missing required argument '-mode'." }},
		{ LogPresetProgramArg::ModeInvalid_err,       { logVerbosity::Error   , "Invalid argument '-mode'." }},
		{ LogPresetProgramArg::GroupMissing_err,      { logVerbosity::Error   , "Missing required argument '-group'." }},
		{ LogPresetProgramArg::GroupInvalid_err,      { logVerbosity::Error   , "Invalid argument '-group'." }},
		{ LogPresetProgramArg::BudgetInvalid_err,     { logVerbosity::Error   , "Budget value for '{0}' invalid." }},
		{ LogPresetProgramArg::BudgetOutOfRange_warn, { logVerbosity::Warning , "Budget value for '{0}' is out of range, defaulting to max: {1}." }},
		{ LogPresetProgramArg::BudgetOptions_log,     { logVerbosity::Log     , "Budget options overview: {0}" }},
		{ LogPresetProgramArg::boolValueInvalid_warn, { logVerbosity::Error   , "'{0}' is not a valid bool value." }},
		{ LogPresetProgramArg::ValidateOptions_log,   { logVerbosity::Log     , "Validation options overview: {0}" }},
	};

	// pre-defined messages for specific logs
	const std::map<LogPresetIo, std::pair<logVerbosity, std::string>> logPresetToMsgIO
	{
		{ LogPresetIo::InPathInvalid_err,         { logVerbosity::Error,   "Invalid filepath '{0}'." }},
		{ LogPresetIo::InPathInvalid_warn,        { logVerbosity::Warning, "Invalid filepath '{0}', skipping..." }},
		{ LogPresetIo::InFileInvalidType_err,     { logVerbosity::Error,   "Invalid file type '{0}.'" }},
		{ LogPresetIo::InFileInvalidType_warn,    { logVerbosity::Warning, "Invalid file type '{0}', skipping..." }},
		{ LogPresetIo::InFileReadFail_err,        { logVerbosity::Error,   "Failed to read file '{0}'." }},
		{ LogPresetIo::InFileReadFail_warn,       { logVerbosity::Warning, "Failed to read file '{0}', skipping..." }},
		{ LogPresetIo::InPathRead_log,            { logVerbosity::Log,     "Reading file '{0}'." }},
		{ LogPresetIo::LogFileAlreadyExists_warn, { logVerbosity::Warning, "Specified log file aleady exists '{0}', appending..." }},
		{ LogPresetIo::LogFileWriteFail_err,      { logVerbosity::Error,   "Failed to write to log file '{0}'." }},
		{ LogPresetIo::LogPathWrite_log,          { logVerbosity::Log,     "Writing to log file '{0}'." }},
		{ LogPresetIo::CsvFileAlreadyExists_warn, { logVerbosity::Warning, "Specified csv file aleady exists '{0}', appending..." }},
		{ LogPresetIo::CsvFileWriteFail_err,      { logVerbosity::Error,   "Failed to write to csv file '{0}'." }},
		{ LogPresetIo::CsvPathWrite_log,          { logVerbosity::Log,     "Writing to csv file '{0}'." }},
	};

	// pre-defined messages for specific logs
	const std::map<LogPresetProcessing, std::pair<logVerbosity, std::string>> logPresetToMsgProcessing
	{
		{ LogPresetProcessing::GenericMissingValues_warn, { logVerbosity::Warning, "Line {0} : Missing expected values on line." }},
		{ LogPresetProcessing::OMalformed_warn,           { logVerbosity::Warning, "Line {0} : Possible malformed object." }},
		{ LogPresetProcessing::ONameMissing_warn,         { logVerbosity::Warning, "Line {0} : Object missing name." }},
		{ LogPresetProcessing::ONameMoreThanOne_warn,     { logVerbosity::Warning, "Line {0} : Object has more than 1 name." }},
		{ LogPresetProcessing::GNameMissing_warn,         { logVerbosity::Warning, "Line {0} : Group missing name." }},
		{ LogPresetProcessing::GNameMoreThanOne_warn,     { logVerbosity::Warning, "Line {0} : Prims belong to multiple groups.Only processing first." }},
		{ LogPresetProcessing::ProcessingStart_log,       { logVerbosity::Log,     "---- Begining File Processing ----" }},
		{ LogPresetProcessing::ProcessingEnd_log,         { logVerbosity::Log,     "---- Finished Processing File ----\n" }},
		{ LogPresetProcessing::ProcessingEndStats_log,    { logVerbosity::Log,     "Processed {0} lines in {1} ms." }},
	};

	const std::string helpMsg = "Help:\n"
		""
		;

	Logger logger;

public:
	LogManager();
	LogManager(const std::filesystem::path& logFilePath);
	~LogManager();

	void enableLoggingToFile(const std::filesystem::path& logFilePath);
	void disableLoggingToFile();

	void logProgramArgs(const int& argc, char* argv[]); // TODO: possible remove or rename

	// logs string with verbosity level
	void log(const std::pair<logVerbosity, std::string>& msg, bool terminateOnError = 1);

	// format and log based on mappings
	void logMsgProgramArg(const LogPresetProgramArg& code, const std::string_view str = {}, const std::string_view defautValue = {});
	// format and log based on mappings
	void logMsgIo(const LogPresetIo& code, const std::filesystem::path filepath = {});
	// format and log based on mappings
	void logMsgProcessing(const LogPresetProcessing& code, const std::string_view value = {});
	// format and log based on mappings
	void logMsgProcessing(const LogPresetProcessing& code, const uint64_t& lineNum);
	// format and log based on mappings
	void logMsgProcessing(const LogPresetProcessing& code, const std::chrono::milliseconds& time, uint64_t& lineCount);
};