#pragma once

#include <iostream>
#include <string>
#include <unordered_set>

// collection of primitive info
class PrimDataCollection
{
	std::unordered_set<size_t> hashedMaterialSet;
public:
	PrimDataCollection(std::string_view objName);

	std::string name;

	uint32_t vertCount = 0;
	uint32_t pointCount = 0; // lone vert or point cloud
	uint32_t lineCount = 0; // lone edge or poly curve

	uint32_t faceTotalCount = 0;
	uint32_t faceTriCount = 0;
	uint32_t faceQuadCount = 0;
	uint32_t faceNgonCount = 0;
	uint16_t subgroupCount = 0;
	// bitfield
	uint8_t hasVertColor : 1 = 0;
	uint8_t hasVertNormals : 1 = 0;
	uint8_t hasUvs : 1 = 0;
	//leftover bits: 5

	// returns number of unique materials in this collection
	size_t getMaterialCount() const;
	// returns true if material is new
	bool addMaterial(std::string_view& materialName);

	bool isEmpty() const;

	// list property values to console
	void debugPrint() const;
};