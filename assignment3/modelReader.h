#pragma once

#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <stdexcept>

// deprecated loader for manual vertices in csv

std::vector<float> readVerticesCSV(const char* filename); // load unique vertices
std::vector<unsigned int> readIndicesCSV(const char* filename); // load triangle vertices index (index buffer stuff)

// object and material loader

struct Vector3 {
	float x;
	float y;
	float z;
};

struct Vector2 {
	float x;
	float y;
};

enum class FaceType;

struct SubMtl
{
	int illuminationModel = 2;

	Vector3 ambientColor{ 1.f,1.f,1.f };
	Vector3 diffuseColor{ 1.f, 1.f, 1.f };
	Vector3 specularColor{ 1.f, 1.f, 1.f };

	float shininess = 64.f;
	float opacity = 1.f;
	float opticalDensity;

	int diffuseColorTextureIdx;
};

struct MaterialFileData
{
	std::map<std::string, int> textureFilenames;
	std::map<std::string, int> materialNames;
	std::vector<SubMtl> materials;
};

struct SubObj
{
	std::string modelObjectName;
	std::string useMaterial;
	std::string smoothShadding;

	std::vector<unsigned int> verticesIdx;
	std::vector<unsigned int> textureMapIdx;
	std::vector<unsigned int> normalsIdx;

	std::vector<float> expandedVertices;
	int expandedVertexLength;
};

struct ObjectFileData
{
	std::string mtlFilename;
	std::string objFilename;

	MaterialFileData mtlFileData;

	std::vector<Vector3> vertices;
	std::vector<Vector2> texCoords;
	std::vector<Vector3> normals;
	std::vector<SubObj> subObjects;
};


// blender object file reader

class ObjFileReader
{
public:
	ObjectFileData read(const char* filename, bool parseMtl = false);
private:

	// main method

	ObjectFileData readObj(const char* filename);
	void expandVertices(ObjectFileData& data);
	void readMtl(ObjectFileData& data);

	// string parser

	bool parse1s(std::stringstream& ss, char delim, std::string& outStr);
	//bool parseIfMatch(std::stringstream& ss, char delim, string compared);
	bool checkEOL(std::stringstream& ss, char delim);
	void parseEOL(std::stringstream& ss, char delim, std::string errStr);

	// float parser

	float parse1f(std::stringstream& ss, char delim, std::string errStr);
	Vector2 parse2f(std::stringstream& ss, char delim, std::string errStr);
	Vector3 parse3f(std::stringstream& ss, char delim, std::string errStr);
	//std::vector<float> parseVecf(std::stringstream& ss, unsigned int n, char delim, std::string errStr);

	// int parser

	bool parse1ui(std::stringstream& ss, char delim, unsigned int& outInt, std::string errStr);

	// special parser

	FaceType parseSubFace(std::stringstream& ss, char delim, std::vector<unsigned int>& outVec, std::string errStr);

	// parser method

	std::string errString(std::string msg, const char* filename, std::string line, int lineCount);
	std::string replaceBasename(std::string filename, std::string basename);

};

