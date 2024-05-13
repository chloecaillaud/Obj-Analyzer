#pragma once
#include <filesystem>
#include <string>
#include <vector>

#include "AssetData.cpp"
using namespace std;
namespace fs = std::filesystem;

struct ValidationBoolElem
{
	// bitfield
	uint8_t shouldCheck   : 1 = 0;
	uint8_t expectedValue : 1 = 0;
	//leftover bits: 6
};

struct ValidationStringElem
{
	uint8_t shouldCheck   : 1 = 0;
	string substring;
};

struct ValidationSettings
{
	ValidationBoolElem containsVerts;
	ValidationBoolElem containsUvs;
	ValidationBoolElem containsVertexNormals;
	ValidationBoolElem containsVertexColor;
	ValidationBoolElem containsLooseEdges;
	ValidationBoolElem containsLoosePoints;
	ValidationBoolElem containsFaces;
	ValidationBoolElem containsTris;
	ValidationBoolElem containsQuads;
	ValidationBoolElem containsNgons;
	ValidationBoolElem containsMaterials;
	ValidationBoolElem MissingName;

	ValidationStringElem namePrefix;
	ValidationStringElem nameSuffix;
};

struct BudgetUint32Elem
{
	uint32_t shouldCheck : 1  = 0;
	uint32_t value       : 31 = 0;
};

struct BudgetSettings {
	BudgetUint32Elem verts;
	BudgetUint32Elem points;
	BudgetUint32Elem lines;
	BudgetUint32Elem faceTotals;
	BudgetUint32Elem faceTris;
	BudgetUint32Elem faceQuads;
	BudgetUint32Elem faceNgons;
	BudgetUint32Elem materials;
	BudgetUint32Elem groups;
};

enum class assetDataGrouping : uint8_t
{
	File,
	Object,
	Vertexgroup
};

enum class ProcessingMode : uint8_t
{
	Overview,
	Validate,
	Budget
};

struct ProcessingSettings
{
	ValidationSettings validations;
	BudgetSettings budgets;

	assetDataGrouping grouping = assetDataGrouping::File;
	ProcessingMode mode = ProcessingMode::Overview;

	fs::path csvFilePath;
	fs::path logFilePath;
	vector<fs::path> inputFilePaths;

	bool multiFile() const
	{
		return bool(inputFilePaths.size() <= 2);
	}

	bool areVertsRelevent() const
	{
		switch (mode)
		{
		case ProcessingMode::Validate:
			return validations.containsVerts.shouldCheck
				|| validations.containsVertexColor.shouldCheck
				|| validations.containsVertexNormals.shouldCheck
				|| validations.containsUvs.shouldCheck
				;
		case ProcessingMode::Budget:
			return budgets.verts.shouldCheck;
		case ProcessingMode::Overview:
			return true;
		}
		return true;
	}

	bool arePointsRelevent() const
	{
		switch (mode)
		{
		case ProcessingMode::Validate:
			return validations.containsLoosePoints.shouldCheck;
		case ProcessingMode::Budget:
			return budgets.points.shouldCheck;
		case ProcessingMode::Overview:
			return true;
		}
		return true;
	}

	bool areLinesRelevent() const
	{
		switch (mode)
		{
		case ProcessingMode::Validate:
			return validations.containsLooseEdges.shouldCheck;
		case ProcessingMode::Budget:
			return budgets.lines.shouldCheck;
		case ProcessingMode::Overview:
			return true;
		}
		return true;
	}

	bool areFacesRelevent() const
	{
		switch (mode)
		{
		case ProcessingMode::Validate:
			return validations.containsFaces.shouldCheck
				|| validations.containsTris.shouldCheck
				|| validations.containsQuads.shouldCheck
				|| validations.containsNgons.shouldCheck
				;
		case ProcessingMode::Budget:
			return budgets.faceTotals.shouldCheck
				|| budgets.faceTris.shouldCheck
				|| budgets.faceQuads.shouldCheck
				|| budgets.faceNgons.shouldCheck
				;
		case ProcessingMode::Overview:
			return true;
		}
		return true;
	}

	bool areMaterialsRelevent() const
	{
		switch (mode)
		{
		case ProcessingMode::Validate:
			return validations.containsMaterials.shouldCheck;
		case ProcessingMode::Budget:
			return budgets.materials.shouldCheck;
		case ProcessingMode::Overview:
			return true;
		}
		return true;
	}

	bool areSubGroupsRelevent() const
	{
		switch (mode)
		{
		case ProcessingMode::Validate:
			return false;
		case ProcessingMode::Budget:
			return budgets.groups.shouldCheck;
		case ProcessingMode::Overview:
			return true;
		}
		return true;
	}
};