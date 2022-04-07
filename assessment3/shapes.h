#pragma once

#include <stdlib.h>
#include <math.h>

#define _USE_MATH_DEFINES
#include <cmath>
#define DEG2RAD(n)	n*(M_PI/180)

float* getMyCircle(int num_segments, float radius, int* arraySize);
