#pragma once

#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <stdexcept>
#include <algorithm>


// more texture file stuff below

struct RawObj
{
	std::string modelObjectName;
	std::string useMaterial;
	std::string smoothShadding;
	std::vector<float> vertices;
	std::vector<float> textureMap;
	std::vector<float> normals;
	std::vector<unsigned int> verticesIdx;
	std::vector<unsigned int> textureMapIdx;
	std::vector<unsigned int> normalsIdx;
	std::vector<float> expandedVertices;
};

struct WholeObj
{
	std::string mtl_file;
	std::vector<RawObj> rawObjects;
};

enum class FaceType;

// deprecated loader for manual vertices in csv

std::vector<float> readVerticesCSV(const char* filename); // load unique vertices
std::vector<unsigned int> readIndicesCSV(const char* filename); // load triangle vertices index

// blender object file reader

class ObjFileReader
{
public:
	WholeObj readObj(const char* filename);
	void expandVertices(WholeObj& data);
private:
	bool parse1s(std::stringstream& ss, std::string& outputString, char delim);
	float parse1f(std::stringstream& ss, std::string errStr, char delim);
	unsigned int parse1ui(std::stringstream& ss, std::string errStr, char delim);
	std::vector<float> parseNf(std::stringstream& ss, unsigned int n, char delim, std::string errStr);
	std::vector<unsigned int> parseFace(std::stringstream& ss, FaceType& type, char delim, std::string errStr);
	void parseEOL(std::stringstream& ss, std::string errStr, char delim);
	std::string errString(std::string msg, const char* filename, std::string line, int lineCount);
};

