#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
using namespace std;

class AssetData
{
	unordered_set<size_t> hashedMaterialSet;
public:
	AssetData(string_view objName) : name(objName) {}

	string name;
	
	uint32_t vertCount  = 0;
	uint32_t pointCount = 0; // lone vert or point cloud
	uint32_t lineCount  = 0; // lone edge or poly curve

	uint32_t faceTotalCount = 0;
	uint32_t faceTriCount   = 0;
	uint32_t faceQuadCount  = 0;
	uint32_t faceNgonCount  = 0;
	uint16_t subgroupCount  = 0;
	// bitfield
	uint8_t hasVertColor       : 1 = 0;
	uint8_t hasVertNormals     : 1 = 0;
	uint8_t hasUvs             : 1 = 0;
	//leftover bits: 5

	size_t getMaterialCount() const
	{
		return hashedMaterialSet.size();
	}

	bool addMaterial(string_view& materialName)
	{
		// return success (ie: material didnt prev exist)
		size_t hashedMaterialName = hash<string_view>{}(materialName);
		return hashedMaterialSet.insert(hashedMaterialName).second;
	}

	void debugPrint() const
	{
		cout
			<< "object name:"      << name               << endl
			<< "vertex count:"     << vertCount          << endl
			<< "point count:"      << pointCount         << endl
			<< "line count:"       << lineCount          << endl
			<< "total face count:" << faceTotalCount     << endl
			<< "tri count:"        << faceTriCount       << endl
			<< "quad count:"       << faceQuadCount      << endl
			<< "Ngon count:"       << faceNgonCount      << endl
			<< "group count:"      << subgroupCount      << endl
			<< "material count:"   << getMaterialCount() << endl
			<< "has vert color:"   << boolalpha << bool(hasVertColor)   << endl
			<< "has vert normals:" << boolalpha << bool(hasVertNormals) << endl
			<< "has uvs:"          << boolalpha << bool(hasUvs)         << endl
			<< endl;
	}

	bool isEmpty() const
	{
		return bool(
			vertCount         == 0
			&& pointCount     == 0
			&& lineCount      == 0
			&& faceTotalCount == 0
			);
	}
};