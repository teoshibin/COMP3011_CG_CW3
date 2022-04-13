#pragma once

#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <stdexcept>
#include <algorithm>

struct Triple {
	float x;
	float y;
	float z;
};

struct Pair {
	float x;
	float y;
};

enum class FaceType;

struct SubObj
{
	std::string modelObjectName;
	std::string useMaterial;
	std::string smoothShadding;
	FaceType attributeType;

	std::vector<unsigned int> verticesIdx;
	std::vector<unsigned int> textureMapIdx;
	std::vector<unsigned int> normalsIdx;
	std::vector<float> expandedVertices;
};

struct WholeObj
{
	std::string mtl_file;
	std::vector<Triple> vertices;
	std::vector<Pair> texCoords;
	std::vector<Triple> normals;
	std::vector<SubObj> subObjects;
};

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
	
	// string parser
	
	bool parse1s(std::stringstream& ss, char delim, std::string& outStr);
	bool parseEOL(std::stringstream& ss, char delim);
	void parseEOL(std::stringstream& ss, char delim, std::string errStr);

	// float parser
	
	float parse1f(std::stringstream& ss, char delim, std::string errStr);
	Pair parse2f(std::stringstream& ss, char delim, std::string errStr);
	Triple parse3f(std::stringstream& ss, char delim, std::string errStr);
	//std::vector<float> parseVecf(std::stringstream& ss, unsigned int n, char delim, std::string errStr);
	
	// int parser

	bool parse1ui(std::stringstream& ss, char delim, unsigned int& outInt, std::string errStr);
	
	// special parser

	FaceType parseSubFace(std::stringstream& ss, char delim, std::vector<unsigned int>& outVec, std::string errStr);

	// general method

	std::string errString(std::string msg, const char* filename, std::string line, int lineCount);
};

