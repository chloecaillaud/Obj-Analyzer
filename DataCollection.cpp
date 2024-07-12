#include "DataCollection.h"

PrimDataCollection::PrimDataCollection(std::string_view objName) : name(objName) {};

size_t PrimDataCollection::getMaterialCount() const
{
	return hashedMaterialSet.size();
}

bool PrimDataCollection::addMaterial(std::string_view& materialName)
{
	size_t hashedMaterialName = std::hash<std::string_view>{}(materialName);
	// .second represents wether inserted entry was new (didn't previously exist)
	return hashedMaterialSet.insert(hashedMaterialName).second;
};

bool PrimDataCollection::isEmpty() const
{
	return bool(
		vertCount         == 0
		&& pointCount     == 0
		&& lineCount      == 0
		&& faceTotalCount == 0
		);
};

void PrimDataCollection::debugPrint() const
{
	std::cout
		<< "object name:" << name << std::endl
		<< "vertex count:" << vertCount << std::endl
		<< "point count:" << pointCount << std::endl
		<< "line count:" << lineCount << std::endl
		<< "total face count:" << faceTotalCount << std::endl
		<< "tri count:" << faceTriCount << std::endl
		<< "quad count:" << faceQuadCount << std::endl
		<< "Ngon count:" << faceNgonCount << std::endl
		<< "group count:" << subgroupCount << std::endl
		<< "material count:" << getMaterialCount() << std::endl
		<< "has vert color:" << std::boolalpha << bool(hasVertColor) << std::endl
		<< "has vert normals:" << std::boolalpha << bool(hasVertNormals) << std::endl
		<< "has uvs:" << std::boolalpha << bool(hasUvs) << std::endl
		<< std::endl;
};
