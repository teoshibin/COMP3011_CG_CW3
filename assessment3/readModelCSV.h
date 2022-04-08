#pragma once

#include <iostream>
#include <string.h>
#include <fstream>
#include <sstream>
#include <vector>

std::vector<float> readVerticesCSV(const char* filename);
std::vector<unsigned int> readIndicesCSV(const char* filename);