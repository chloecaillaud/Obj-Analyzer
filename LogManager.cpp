#pragma once
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstdarg>
#include <format>
#include <chrono>
#include <exception>

using namespace std;
namespace fs = std::filesystem;

enum class logVerbosity : uint8_t
{
	None,
	Log,
	Warning,
	Error,
};

enum class ProgramArgLogs : uint8_t
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

enum class IoLogs : uint8_t
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

enum class ProcessingLogs : uint8_t
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
	ofstream fileWriteStream;
	bool bWriteToFile = false;
	bool bWriteToConsole = false;

public:
	Logger(bool enableWriteToConsole = true) : bWriteToConsole(enableWriteToConsole) {}
	~Logger()
	{
		close();
	}

	void open(const fs::path& filepath, ios_base::openmode openmode)
	{
		fileWriteStream.open(filepath, openmode);
		if (fileWriteStream.is_open())
		{
			bWriteToFile = true;
		}
	}

	void close()
	{
		fileWriteStream.close();
		bWriteToFile = false;
	}

	template<typename T>
	Logger& operator<<(const T& data)
	{
		if (bWriteToFile)
		{
			fileWriteStream << data;
		}
		if (bWriteToConsole)
		{
			cout << data;
		}
		return *this;
	}

	// defined functionaly for custom endl
	typedef Logger& (*LoggerManip)(Logger&);
	Logger& operator<<(LoggerManip manip)
	{
		return manip(*this);
	}

	// custom endl
	static Logger& endl(Logger& logger)
	{
		if (logger.bWriteToFile)
		{
			logger.fileWriteStream << std::endl;
		}
		if (logger.bWriteToConsole)
		{
			cout << std::endl;
		}
		return logger;
	}
};

class LoggedErrorException
{
	const string msg;
public:
	LoggedErrorException(const string& msg)
		: msg(msg)
	{};

	const char* what() const
	{
		return msg.c_str();
	}
};

class LogManager
{
	const map<ProgramArgLogs, pair<logVerbosity, string>> programArgLogMsgs
	{
		{ ProgramArgLogs::GenericMissing_err,    { logVerbosity::Error   , "Missing argument '{0}'." }},
		{ ProgramArgLogs::GenericMissing_warn,   { logVerbosity::Warning , "Missing argument '{0}', defaulting to {1}." }},
		{ ProgramArgLogs::GenericInvaid_err,     { logVerbosity::Error   , "Invalid argument '{0}'" }},
		{ ProgramArgLogs::GenericInvaid_warn,    { logVerbosity::Warning , "Invalid argument '{0}', defaulting to {1}." }},
		{ ProgramArgLogs::InFilepathMissing_err, { logVerbosity::Error   , "No valid file(s) to analize specified." }},
		{ ProgramArgLogs::ModeMissing_err,       { logVerbosity::Error   , "Missing required argument '-mode'." }},
		{ ProgramArgLogs::ModeInvalid_err,       { logVerbosity::Error   , "Invalid argument '-mode'." }},
		{ ProgramArgLogs::GroupMissing_err,      { logVerbosity::Error   , "Missing required argument '-group'." }},
		{ ProgramArgLogs::GroupInvalid_err,      { logVerbosity::Error   , "Invalid argument '-group'." }},
		{ ProgramArgLogs::BudgetInvalid_err,     { logVerbosity::Error   , "Budget value for '{0}' invalid." }},
		{ ProgramArgLogs::BudgetOutOfRange_warn, { logVerbosity::Warning , "Budget value for '{0}' is out of range, defaulting to max: {1}." }},
		{ ProgramArgLogs::BudgetOptions_log,     { logVerbosity::Log     , "Budget options overview: {0}" }},
		{ ProgramArgLogs::boolValueInvalid_warn, { logVerbosity::Error   , "'{0}' is not a valid bool value." }},
		{ ProgramArgLogs::ValidateOptions_log,   { logVerbosity::Log     , "Validation options overview: {0}" }},
	};

	const map<IoLogs, pair<logVerbosity, string>> ioLogMsgs
	{
		{ IoLogs::InPathInvalid_err,         { logVerbosity::Error,   "Invalid filepath '{0}'." }},
		{ IoLogs::InPathInvalid_warn,        { logVerbosity::Warning, "Invalid filepath '{0}', skipping..." }},
		{ IoLogs::InFileInvalidType_err,     { logVerbosity::Error,   "Invalid file type '{0}.'" }},
		{ IoLogs::InFileInvalidType_warn,    { logVerbosity::Warning, "Invalid file type '{0}', skipping..." }},
		{ IoLogs::InFileReadFail_err,        { logVerbosity::Error,   "Failed to read file '{0}'." }},
		{ IoLogs::InFileReadFail_warn,       { logVerbosity::Warning, "Failed to read file '{0}', skipping..." }},
		{ IoLogs::InPathRead_log,            { logVerbosity::Log,     "Reading file '{0}'." }},
		{ IoLogs::LogFileAlreadyExists_warn, { logVerbosity::Warning, "Specified log file aleady exists '{0}', appending..." }},
		{ IoLogs::LogFileWriteFail_err,      { logVerbosity::Error,   "Failed to write to log file '{0}'." }},
		{ IoLogs::LogPathWrite_log,          { logVerbosity::Log,     "Writing to log file '{0}'." }},
		{ IoLogs::CsvFileAlreadyExists_warn, { logVerbosity::Warning, "Specified csv file aleady exists '{0}', appending..." }},
		{ IoLogs::CsvFileWriteFail_err,      { logVerbosity::Error,   "Failed to write to csv file '{0}'." }},
		{ IoLogs::CsvPathWrite_log,          { logVerbosity::Log,     "Writing to csv file '{0}'." }},
	};

	const map<ProcessingLogs, pair<logVerbosity, string>> processingLogMsgs
	{
		{ ProcessingLogs::GenericMissingValues_warn, { logVerbosity::Warning, "Line {0} : Missing expected values on line." }},
		{ ProcessingLogs::OMalformed_warn,           { logVerbosity::Warning, "Line {0} : Possible malformed object." }},
		{ ProcessingLogs::ONameMissing_warn,         { logVerbosity::Warning, "Line {0} : Object missing name." }},
		{ ProcessingLogs::ONameMoreThanOne_warn,     { logVerbosity::Warning, "Line {0} : Object has more than 1 name." }},
		{ ProcessingLogs::GNameMissing_warn,         { logVerbosity::Warning, "Line {0} : Group missing name." }},
		{ ProcessingLogs::GNameMoreThanOne_warn,     { logVerbosity::Warning, "Line {0} : Prims belong to multiple groups.Only processing first." }},
		{ ProcessingLogs::ProcessingStart_log,       { logVerbosity::Log,     "---- Begining File Processing ----" }},
		{ ProcessingLogs::ProcessingEnd_log,         { logVerbosity::Log,     "---- Finished Processing File ----\n" }},
		{ ProcessingLogs::ProcessingEndStats_log,    { logVerbosity::Log,     "Processed {0} lines in {1} ms." }},
	};

	Logger logger;
public:
	LogManager() {};
	LogManager(const fs::path& logFilePath)
	{
		if (!logFilePath.empty())
		{
			logger.open(logFilePath, ios::app);
		}
	};
	~LogManager() { logger.close(); };

	void enableLoggingToFile(const fs::path& logFilePath)
	{
		if (!logFilePath.empty())
		{
			logger.open(logFilePath, ios::app);
		}
	};

	void disableLoggingToFile()
	{
		logger.close();
	};

	void logProgramArgs(const int& argc, char* argv[])
	{
		logger << "Cmd arguments: ";
		for (int i = 1; i < argc; i++)
		{
			// 1 <= i <= argc-1 since 0 is prog name and argc is null term.
			logger << argv[i] << ' ';
		}
		logger << logger.endl;
	}

	void logMsg(const pair<logVerbosity, string>& msg, bool terminateOnError = 1)
	{
		switch (msg.first)
		{
		case logVerbosity::Log:
			logger << "Log: ";
			break;
		case logVerbosity::Warning:
			logger << "Warning: ";
			break;
		case logVerbosity::Error:
			logger << "Error: ";
			break;
		default:
			break;
		}
		logger << msg.second << logger.endl;

		if (terminateOnError && msg.first == logVerbosity::Error)
		{
			throw LoggedErrorException("\nError occurred, see logs for details. \n Exiting...");
		}
	}

	void logProgramArgMsg(const ProgramArgLogs& code, const string_view str = {}, const string_view defautValue = {})
	{
		auto it = programArgLogMsgs.find(code);
		if (it != programArgLogMsgs.end())
		{
			const pair<logVerbosity, string> formattedMsg =
			{
				it->second.first,
				vformat(string_view(it->second.second), make_format_args(str, defautValue))
			};
			logMsg(formattedMsg);
		}
		else
		{
			logMsg({ logVerbosity::Error, "Unknown argument error." });
		}
	}


	void logIoMsg(const IoLogs& code, const fs::path filepath = {})
	{
		auto it = ioLogMsgs.find(code);
		if (it != ioLogMsgs.end())
		{
			const pair<logVerbosity, string> formattedMsg =
			{
				it->second.first,
				vformat(string_view(it->second.second), make_format_args(filepath.generic_string()))
			};
			logMsg(formattedMsg);
		}
		else
		{
			logMsg({ logVerbosity::Error, "Unknown io error." });
		}
	}

	void logProcessingMsg(const ProcessingLogs& code, const string_view value = {})
	{
		auto it = processingLogMsgs.find(code);
		if (it != processingLogMsgs.end())
		{
			const pair<logVerbosity, string> formattedMsg =
			{
				it->second.first,
				vformat(string_view(it->second.second), make_format_args(value))
			};
			logMsg(formattedMsg);
		}
		else
		{
			logMsg({ logVerbosity::Error, "Unknown processing error." });
		}
	}

	void logProcessingMsg(const ProcessingLogs& code, const uint64_t& lineNum)
	{
		auto it = processingLogMsgs.find(code);
		if (it != processingLogMsgs.end())
		{
			const pair<logVerbosity, string> formattedMsg =
			{
				it->second.first,
				vformat(string_view(it->second.second), make_format_args(lineNum))
			};
			logMsg(formattedMsg);
		}
		else
		{
			logMsg({ logVerbosity::Error, format("Line {0}: Unknown processing error.", lineNum) });
		}
	}

	void logProcessingMsg(const ProcessingLogs& code, const chrono::milliseconds& time, uint64_t& lineCount)
	{
		auto it = processingLogMsgs.find(code);
		if (it != processingLogMsgs.end())
		{
			const pair<logVerbosity, string> formattedMsg =
			{
				it->second.first,
				vformat(string_view(it->second.second), make_format_args(lineCount, time))
			};
			logMsg(formattedMsg);
		}
		else
		{
			logMsg({ logVerbosity::Error, "Unknown processing error." });
		}
	}
};