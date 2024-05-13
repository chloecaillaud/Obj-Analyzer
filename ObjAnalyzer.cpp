#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include "Processors.cpp"
#include "OutputHandlers.cpp"
#include "LogManager.cpp"
using namespace std;
namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
	LogManager loggingManager;
	CmdArgParcer argParser(argc, argv, loggingManager);

	if (argParser.hasHelpArg())
	{
		// TODO: output help
		return 0;
	}
	loggingManager.enableLoggingToFile(argParser.getLogPath());


	try {
		const ProcessingSettings settings = argParser.asSettings();

		unique_ptr<ResultOutputterBase> outputter;
		switch (settings.mode)
		{
		case ProcessingMode::Overview: outputter = make_unique<ResultOutputterOverview>(settings, loggingManager); break;
		case ProcessingMode::Validate: outputter = make_unique<ResultOutputterValidate>(settings, loggingManager); break;
		case ProcessingMode::Budget:   outputter = make_unique<ResultOutputterBudget>(settings, loggingManager); break;
		}

		for (const fs::path& filepath : settings.inputFilePaths)
		{
			FileProcessor_Obj processor(filepath, settings, loggingManager);
			processor.processFile();

			if (outputter)
			{
				for (const AssetData& asset : processor.assets)
				{
					outputter->outputReports(asset);
				}
			}
		}
		return 0;
	}
	catch (LoggedErrorException e)
	{
		cout << e.what();
		return 1;
	}
};