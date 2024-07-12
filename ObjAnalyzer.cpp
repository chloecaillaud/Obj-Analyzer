#include "ObjAnalyzer.h"

int main(int argc, char* argv[])
{
	// create the logging manager first to allow logging of arg issues
	LogManager loggingManager;
	ProgramArgParcer argParser(argc, argv, loggingManager);

	if (argParser.hasHelpArg())
	{
		// TODO: output help
		return 0;
	}
	loggingManager.enableLoggingToFile(argParser.getLogPath());

	try
	{
		const ProcessingSettings settings = argParser.asSettings();

		std::unique_ptr<ResultOutputterBase> outputFormatter;
		switch (settings.mode)
		{
		case ProcessingMode::Overview: outputFormatter = std::make_unique<ResultOutputterOverview>(settings, loggingManager); break;
		case ProcessingMode::Validate: outputFormatter = std::make_unique<ResultOutputterValidate>(settings, loggingManager); break;
		case ProcessingMode::Budget:   outputFormatter = std::make_unique<ResultOutputterBudget>(settings, loggingManager); break;
		}

		for (const std::filesystem::path& filepath : settings.inputFilePaths)
		{
			FileProcessor processor(filepath, settings, loggingManager);
			processor.processFile();

			if (outputFormatter)
			{
				for (const PrimDataCollection& asset : processor.PrimDataCollections)
				{
					outputFormatter->outputReports(asset);
				}
			}
		}
		return 0;
	}
	catch (LoggedErrorException e)
	{
		std::cout << e.what();
		return 1;
	};
};