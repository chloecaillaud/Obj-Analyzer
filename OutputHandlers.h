#pragma once

#include <filesystem>
#include <fstream>
#include <vector>
#include <format>
#include <iostream>

#include "Settings.h"
#include "LogManager.h"

class ResultOutputterBase
{
protected:
	const ProcessingSettings& settings;
	LogManager& loggingManager;
public:
	ResultOutputterBase(const ProcessingSettings& settings, LogManager& loggingManager);
	virtual ~ResultOutputterBase() = default;

	virtual void outputReports(const PrimDataCollection&) = 0;
	virtual void outputToLog(const PrimDataCollection&) = 0;
	virtual void outputToCsv(const PrimDataCollection&, const std::filesystem::path&, const char = ',') = 0;
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

	const std::string generateTxtFormattedReport(const PrimDataCollection& asset) const;

	void outputToLog(const std::string& txtReport);

public:
	ResultOutputterOverview(const ProcessingSettings& settings, LogManager& loggingManager);

	void outputReports(const PrimDataCollection& asset) override;

	void outputToCsv(const PrimDataCollection& asset, const std::filesystem::path& filepath, const char sep = ',') override;
	void outputToLog(const PrimDataCollection& asset) override;
};

class ResultOutputterValidate : public ResultOutputterBase
{
	const std::string makeLineFromBoolElem(const ValidationBoolElem& validationElem, const bool statValue, const std::string& statName) const;

	const std::string generateTxtFormattedReport(const PrimDataCollection& asset) const;

	inline const std::string makeCsvValueFromValidationElem(const ValidationBoolElem& validationElem, const bool statValue) const;
	inline const std::string makeCsvValueFromValidationElem(const ValidationStringElem& validationElem, const bool isValid) const;

	void outputToConsole(const std::string& txtReport);
	void outputToLog(const std::string& txtReport);

public:
	ResultOutputterValidate(const ProcessingSettings& settings, LogManager& loggingManager);

	void outputReports(const PrimDataCollection& asset) override;

	void outputToCsv(const PrimDataCollection& asset, const std::filesystem::path& filepath, const char sep = ',') override;
	void outputToLog(const PrimDataCollection& asset) override;
};

class ResultOutputterBudget : public ResultOutputterBase
{
	const std::string makeLineFromBudgetElem(const BudgetUint32Elem& budgetElem, const uint32_t statValue, const std::string& statName) const;

	const std::string generateTxtFormattedReport(const PrimDataCollection& asset) const;

	const std::string makeCsvValueFromBudgetElem(const BudgetUint32Elem& budgetElem, const uint32_t statValue) const;

	void outputToConsole(const std::string& txtReport);
	void outputToLog(const std::string& txtReport);

public:
	ResultOutputterBudget(const ProcessingSettings& settings, LogManager& loggingManager);

	void outputReports(const PrimDataCollection& asset) override;

	void outputToCsv(const PrimDataCollection& asset, const std::filesystem::path& filepath, const char sep = ',') override;
	void outputToLog(const PrimDataCollection& asset) override;
};