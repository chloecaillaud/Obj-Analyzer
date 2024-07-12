#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "DataCollection.h"

struct ValidationBoolElem
{
	// bitfield
	uint8_t shouldCheck : 1 = 0;
	uint8_t expectedValue : 1 = 0;
	// leftover bits: 6
};

struct ValidationStringElem
{
	uint8_t shouldCheck : 1 = 0;
	// leftover bits: 7
	std::string substring;
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
	// pack bool into bitfield to prevent wasting 31 bits
	uint32_t shouldCheck : 1 = 0;
	uint32_t value : 31 = 0;
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

enum class DataCollectionGrouping : uint8_t
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

	DataCollectionGrouping grouping = DataCollectionGrouping::File;
	ProcessingMode mode = ProcessingMode::Overview;

	std::filesystem::path csvFilePath;
	std::filesystem::path logFilePath;
	std::vector<std::filesystem::path> inputFilePaths;

	bool isMultiFile() const;

	bool areVertsRelevent() const;
	bool arePointsRelevent() const;
	bool areLinesRelevent() const;
	bool areFacesRelevent() const;
	bool areMaterialsRelevent() const;
	bool areSubGroupsRelevent() const;
};