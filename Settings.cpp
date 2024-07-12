#include "Settings.h"

bool ProcessingSettings::isMultiFile() const
{
	return bool(inputFilePaths.size() <= 2);
};

bool ProcessingSettings::areVertsRelevent() const
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
};

bool ProcessingSettings::arePointsRelevent() const
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
};

bool ProcessingSettings::areLinesRelevent() const
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
};

bool ProcessingSettings::areFacesRelevent() const
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
};

bool ProcessingSettings::areMaterialsRelevent() const
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
};

bool ProcessingSettings::areSubGroupsRelevent() const
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
};