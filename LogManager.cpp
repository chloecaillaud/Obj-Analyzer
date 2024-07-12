#include "LogManager.h"

Logger::Logger(bool enableWriteToConsole) : bWriteToConsole(enableWriteToConsole) {};

Logger::~Logger()
{
	close();
};

void Logger::open(const std::filesystem::path& filepath, std::ios_base::openmode openmode)
{
	fileWriteStream.open(filepath, openmode);
	if (fileWriteStream.is_open())
	{
		bWriteToFile = true;
	}
	else
	{
		// TODO: log error to console
	}
};

void Logger::close()
{
	fileWriteStream.close();
	bWriteToFile = false;
};

template<typename T>
Logger& Logger::operator<<(const T& data)
{
	if (bWriteToFile)
	{
		fileWriteStream << data;
	}
	if (bWriteToConsole)
	{
		std::cout << data;
	}
	return *this;
};

typedef Logger& (*LoggerManip)(Logger&);
Logger& Logger::operator<<(LoggerManip manip)
{
	return manip(*this);
};

Logger& Logger::endl(Logger& logger) //TODO: confirm no need for static
{
	if (logger.bWriteToFile)
	{
		logger.fileWriteStream << std::endl;
	}
	if (logger.bWriteToConsole)
	{
		std::cout << std::endl;
	}
	return logger;
};

// --------------------------------
LoggedErrorException::LoggedErrorException(const std::string& msg) : msg(msg) {};

const char* LoggedErrorException::what() const
{
	return msg.c_str();
};

// --------------------------------
LogManager::LogManager() {};
LogManager::LogManager(const std::filesystem::path& logFilePath)
{
	if (!logFilePath.empty())
	{
		logger.open(logFilePath, std::ios::app);
	}
};

LogManager::~LogManager() { logger.close(); };

void LogManager::enableLoggingToFile(const std::filesystem::path& logFilePath)
{
	if (!logFilePath.empty())
	{
		logger.open(logFilePath, std::ios::app);
	}
};

void LogManager::disableLoggingToFile()
{
	logger.close();
};

void LogManager::log(const std::pair<logVerbosity, std::string>& msg, bool terminateOnError)
{
	// prefix with log type
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
	// log actual message
	logger << msg.second << logger.endl;

	if (terminateOnError && msg.first == logVerbosity::Error)
	{
		throw LoggedErrorException("\nError occurred, see logs for details. \n Exiting...");
	}
};

void LogManager::logProgramArgs(const int& argc, char* argv[]) // TODO: replace with logging after progessing?
{
	logger << "Cmd arguments: ";
	for (int i = 1; i < argc; i++)
	{
		// 1 <= i <= argc-1 since 0 is prog name and argc is null term.
		logger << argv[i] << ' ';
	}
	logger << logger.endl;
};

void LogManager::logMsgProgramArg(const LogPresetProgramArg& code, const std::string_view str, const std::string_view defautValue)
{
	auto it = logPresetToMsgProgramArg.find(code);
	if (it != logPresetToMsgProgramArg.end())
	{
		const std::pair<logVerbosity, std::string> formattedMsg =
		{
			it->second.first,
			vformat(std::string_view(it->second.second), make_format_args(str, defautValue))
		};
		log(formattedMsg);
	}
	else
	{
		log({ logVerbosity::Error, "Unknown argument error." });
	}
};

void LogManager::logMsgIo(const LogPresetIo& code, const std::filesystem::path filepath)
{
	auto it = logPresetToMsgIO.find(code);
	if (it != logPresetToMsgIO.end())
	{
		const std::pair<logVerbosity, std::string> formattedMsg =
		{
			it->second.first,
			vformat(std::string_view(it->second.second), make_format_args(filepath.generic_string()))
		};
		log(formattedMsg);
	}
	else
	{
		log({ logVerbosity::Error, "Unknown io error." });
	}
};

void LogManager::logMsgProcessing(const LogPresetProcessing& code, const std::string_view value)
{
	auto it = logPresetToMsgProcessing.find(code);
	if (it != logPresetToMsgProcessing.end())
	{
		const std::pair<logVerbosity, std::string> formattedMsg =
		{
			it->second.first,
			vformat(std::string_view(it->second.second), make_format_args(value))
		};
		log(formattedMsg);
	}
	else
	{
		log({ logVerbosity::Error, "Unknown processing error." });
	}
};

void LogManager::logMsgProcessing(const LogPresetProcessing& code, const uint64_t& lineNum)
{
	auto it = logPresetToMsgProcessing.find(code);
	if (it != logPresetToMsgProcessing.end())
	{
		const std::pair<logVerbosity, std::string> formattedMsg =
		{
			it->second.first,
			std::vformat(std::string_view(it->second.second), std::make_format_args(lineNum))
		};
		log(formattedMsg);
	}
	else
	{
		log({ logVerbosity::Error, std::format("Line {0}: Unknown processing error.", lineNum) });
	}
};

void LogManager::logMsgProcessing(const LogPresetProcessing& code, const std::chrono::milliseconds& time, uint64_t& lineCount)
{
	auto it = logPresetToMsgProcessing.find(code);
	if (it != logPresetToMsgProcessing.end())
	{
		const std::pair<logVerbosity, std::string> formattedMsg =
		{
			it->second.first,
			std::vformat(std::string_view(it->second.second), std::make_format_args(lineCount, time))
		};
		log(formattedMsg);
	}
	else
	{
		log({ logVerbosity::Error, "Unknown processing error." });
	}
};