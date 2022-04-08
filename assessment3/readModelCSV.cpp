
#include "readModelCSV.h"

//std::vector<float> readCSVFloat(const char* filename, int length_per_row)
//{
//	std::ifstream inputFile;
//	inputFile.open(filename);
//
//	std::string line = "";
//	std::vector<float> vertices;
//	while (std::getline(inputFile, line))
//	{
//		std::stringstream inputString(line);
//
//		for (int i = 0; i < length_per_row; i++)
//		{
//			std::string tempString = "";
//			std::getline(inputString,tempString,',');
//			float value = std::stof(tempString.c_str());
//			vertices.push_back(value);
//		}
//
//		line = "";
//
//	}
//
//	return vertices;
//}

std::vector<float> readVerticesCSV(const char* filename)
{
	std::ifstream inputFile;
	inputFile.open(filename);

	std::string line = "";
	std::vector<float> vertices;
	while (std::getline(inputFile, line))
	{
		std::stringstream inputString(line);

		std::string tempString = "";
		while (std::getline(inputString, tempString, ','))
		{
			float value = std::stof(tempString.c_str());
			vertices.push_back(value);
			tempString = "";
		}

		line = "";

	}

	return vertices;
}

std::vector<unsigned int> readIndicesCSV(const char* filename)
{
	std::ifstream inputFile;
	inputFile.open(filename);

	std::string line = "";
	std::vector<unsigned int> indices;
	while (std::getline(inputFile, line))
	{
		std::stringstream inputString(line);

		std::string tempString = "";
		while (std::getline(inputString, tempString, ','))
		{
			unsigned int value = std::stoi(tempString.c_str());
			indices.push_back(value);
			tempString = "";
		}

		line = "";

	}

	return indices;
}