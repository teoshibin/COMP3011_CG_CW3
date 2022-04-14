#pragma once

#include <stdlib.h>
#include <math.h>
#define _USE_MATH_DEFINES
#include <cmath>
#define DEG2RAD(n)	n*(M_PI/180)
#include <vector>

std::vector<float> getSkyboxCube();
std::vector<float> getRectangle();
std::vector<float> getCircle(int num_segments, float radius);
