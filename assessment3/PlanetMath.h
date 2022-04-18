#pragma once

#include <vector>
#include <map>
#include <string>

#ifndef MYLIB_CONSTANTS_H
#define MYLIB_CONSTANTS_H

namespace PConst
{
	// PLEASE REFER TO THE "BodiesConstants" SPREADSHEET WHERE I SORT ALL OF THESE CONSTANTS
	// radius is in km
	// periods are in earth days or specific planet days
	// inclination with respect to ecliptic plane in degrees
	// axial tilt in degrees (note some self spin clockwise aka retrograde)
	// ascending node angle on planet orbit plane in degrees

	// SUN
	const float SUN_RADIUS = 696340.f;
	const float SUN_AXIAL_TILT = 7.25f;

	// MERCURY
	const float MERCURY_RADIUS = 2439.7f;
	const float MERCURY_ORBITAL_PERIOD = 87.9691f;
	const float MERCURY_LOCAL_ORBITAL_PERIOD = 1.500005977f;
	const float MERCURY_ASCENDING_NODE = 48.331f;
	const float MERCURY_INCLINATION = 7.005f;
	const float MERCURY_AXIAL_TILT = 0.034f;

	// VENUS
	const float VENUS_RADIUS = 6051.8f;
	const float VENUS_ORBITAL_PERIOD = 224.701f;
	const float VENUS_LOCAL_ORBITAL_PERIOD = 1.924633833f;
	const float VENUS_ASCENDING_NODE = 76.68f;
	const float VENUS_INCLINATION = 3.39458f;
	const float VENUS_AXIAL_TILT = 177.36f; // retrograde

	// EARTH
	const float EARTH_RADIUS = 6371.f;
	const float EARTH_ORBITAL_PERIOD = 365.256363004f;
	const float EARTH_ASCENDING_NODE = -11.26064f;
	const float EARTH_INCLINATION = 0.f;
	const float EARTH_AXIAL_TILT = 23.4392811f;

	// MOON
	const float MOON_RADIUS = 1737.4f;
	const float MOON_ORBITAL_PERIOD = 27.321661f;
	const float MOON_LOCAL_ORBITAL_PERIOD = 1.f;
	//const float MOON_ASCENDING_NODE; // Not constant: Regressing by one revolution in 18.61 years
	const float MOON_INCLINATION = 5.145f;
	const float MOON_AXIAL_TILT = 6.687f;
	
	// MARS
	const float MARS_RADIUS = 3389.5f; // km
	const float MARS_ORBITAL_PERIOD = 686.98f;
	const float MARS_LOCAL_ORBITAL_PERIOD = 669.7709063f;
	const float MARS_ASCENDING_NODE = 49.558f;
	const float MARS_INCLINATION = 1.85f;
	const float MARS_AXIAL_TILT = 25.19f;

	// JUPITER
	const float JUPITER_RADIUS = 69911.f;
	const float JUPITER_ORBITAL_PERIOD = 4332.59f;
	const float JUPITER_LOCAL_ORBITAL_PERIOD = 10467.99987f;
	const float JUPITER_ASCENDING_NODE = 100.464f;
	const float JUPITER_INCLINATION = 1.303f;
	const float JUPITER_AXIAL_TILT = 3.12f;

	// SATURN
	const float SATURN_RADIUS = 58232.f;
	const float SATURN_ORBITAL_PERIOD = 10759.22f;
	const float SATURN_LOCAL_ORBITAL_PERIOD = 24132.84795f;
	const float SATURN_ASCENDING_NODE = 113.665f;
	const float SATURN_INCLINATION = 2.485f;
	const float SATURN_AXIAL_TILT = 26.73f;

	// URANUS
	const float URANUS_RADIUS = 25362.f;
	const float URANUS_ORBITAL_PERIOD = 30688.5f;
	const float URANUS_LOCAL_ORBITAL_PERIOD = 42738.3101f;
	const float URANUS_ASCENDING_NODE = 74.006f;
	const float URANUS_INCLINATION = 0.773f;
	const float URANUS_AXIAL_TILT = 97.77f;

	// NEPTUNE
	const float NEPTUNE_RADIUS = 24622.f;
	const float NEPTUNE_ORBITAL_PERIOD = 60195.f;
	const float NEPTUNE_LOCAL_ORBITAL_PERIOD = 89731.72161f;
	const float NEPTUNE_ASCENDING_NODE = 131.783f;
	const float NEPTUNE_INCLINATION = 1.77f;
	const float NEPTUNE_AXIAL_TILT = 28.33f;

	// PLUTO
	const float PLUTO_RADIUS = 1188.3f;
	const float PLUTO_ORBITAL_PERIOD = 90560.f;
	const float PLUTO_LOCAL_ORBITAL_PERIOD = 14150.f;
	const float PLUTO_ASCENDING_NODE = 110.299f;
	const float PLUTO_INCLINATION = 17.16f;
	const float PLUTO_AXIAL_TILT = 122.53f;
}
#endif

struct BodyConst
{
	float radius;
	float orbitalPeriod;
	float localOrbitalPeriod;
	float ascendingNode;
	float inclination;
	float axialTilt;
};

struct RenderedBody
{
	// === render ===
	
	// body orbiting position
	std::vector<float> position;

	// body spin rotation
	float rotation = 0;

	// special variable to fix nested orbit object (such as moon) not 
	// following ascending node of parent orbiting object 
	// the result of not using this variable simply makes moon orbiting an 
	// empty space while earth is shifted to it's ascending node angle
	float parentsAscendingNodeSum = 0; // this will be calculated using recursion
	float allInclinationSum = 0; // make object stay perpendicular to it's orbit plane after the orbit is inclined

	// animation
	float scale = 1;
	float scaleModifier = 1;
	float distanceMargin = 0;
	float orbitRadius = 0;
	float ovalRatio = 1;

	// linking
	int bodyConstantIdx = -1;
	int animatorIndex = -1;
	int orbitParentIdx = -1;
	int VAOIdx = -1;
};

class PlanetMath
{
public:
	std::vector<BodyConst> getSolarSystemConstants();
	int getPlanet(std::string name);
	float getRelativeValue(float a1, float b1, float b2);
	float getRelativeValue(float a1, float b1, float b2, float r);
	//float getMarginedRadius(float radiusA, float radiusB, float marginB, float modifierB);
	//bool findEqual(std::vector<float> values, float value);
	//std::vector<float> batchGetRelativeValue(std::vector<float> as1, float bs1, float bs2);
	float sumAllInclinations(std::vector<RenderedBody> rb, std::vector<BodyConst> bc, int targetIndex);
	float sumAllAscendingNodes(std::vector<RenderedBody> rb, std::vector<BodyConst> bc, int targetIndex);
private:
	std::map < std::string, int> keyMap
	{
		{"sun",		1},
		{"mercury",	2},
		{"venus",	3},
		{"earth",	4},
		{"mars",	5},
		{"jupiter",	6},
		{"saturn",	7},
		{"uranus",	8},
		{"neptune",	9},
		{"pluto",	10},
		{"moon",	11},
	};
};
