
#include "PlanetMath.h"


/// <summary>
/// calculate a2 using ratio
/// </summary>
/// <param name="a1">object 1 value 1</param>
/// <param name="b1">object 2 value 1</param>
/// <param name="b2">object 2 value 2</param>
/// <returns>object 1 value 2</returns>
float PlanetMath::getRelativeValue(float a1, float b1, float b2)
{
	return a1 / b1 * b2;
}

/// <summary>
/// calculate a2 using ratio then multiply it with r
/// </summary>
/// <param name="a1">object 1 value 1</param>
/// <param name="b1">object 2 value 1</param>
/// <param name="b2">object 2 value 2</param>
/// <returns>object 1 value 2</returns>
float PlanetMath::getRelativeValue(float a1, float b1, float b2, float r)
{
	return a1 / b1 * b2 * r;
}

/// <summary>
/// calculate scene object radius and apply custom modifier
/// this is mostly used to generate orbit radius
/// </summary>
/// <param name="radius1">object 1 radius</param>
/// <param name="radius2">object 2 radius</param>
/// <param name="scale2">object 2 scale</param>
/// <returns>object 1 scale</returns>
float PlanetMath::getScaledRadiusDistance(float scale1, float scale2, float sceneObjectRadius, float modifier)
{
	return (scale1 * sceneObjectRadius + scale2 * sceneObjectRadius) * modifier;
}

float earthDayToLocalDay(float earthDays, float localDayLength)
{
	return earthDays / localDayLength;
}