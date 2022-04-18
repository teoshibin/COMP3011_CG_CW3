
#include "PlanetMath.h"

//template<typename K, typename V>
//static std::map<V, K> reverse_map(const std::map<K, V>& m) {
//	map<V, K> r;
//	for (const auto& kv : m)
//		r[kv.second] = kv.first;
//	return r;
//}

std::vector<BodyConst> PlanetMath::getSolarSystemConstants()
{
	std::vector<BodyConst> animBodies(11);
	
	// bodies that orbit the sun

	animBodies[0].radius = PConst::SUN_RADIUS;
	animBodies[0].orbitalPeriod = INFINITY;
	animBodies[0].localOrbitalPeriod = INFINITY;
	animBodies[0].ascendingNode = 0;
	animBodies[0].inclination = 0;
	animBodies[0].axialTilt = PConst::SUN_AXIAL_TILT;

	animBodies[1].radius = PConst::MERCURY_RADIUS;
	animBodies[1].orbitalPeriod = PConst::MERCURY_ORBITAL_PERIOD;
	animBodies[1].localOrbitalPeriod = PConst::MERCURY_LOCAL_ORBITAL_PERIOD;
	animBodies[1].ascendingNode = PConst::MERCURY_ASCENDING_NODE;
	animBodies[1].inclination = PConst::MERCURY_INCLINATION;
	animBodies[1].axialTilt = PConst::MERCURY_AXIAL_TILT;

	animBodies[2].radius = PConst::VENUS_RADIUS;
	animBodies[2].orbitalPeriod = PConst::VENUS_ORBITAL_PERIOD;
	animBodies[2].localOrbitalPeriod = PConst::VENUS_LOCAL_ORBITAL_PERIOD;
	animBodies[2].ascendingNode = PConst::VENUS_ASCENDING_NODE;
	animBodies[2].inclination = PConst::VENUS_INCLINATION;
	animBodies[2].axialTilt = PConst::VENUS_AXIAL_TILT;
	
	animBodies[3].radius = PConst::EARTH_RADIUS;
	animBodies[3].orbitalPeriod = PConst::EARTH_ORBITAL_PERIOD;
	animBodies[3].localOrbitalPeriod = PConst::EARTH_ORBITAL_PERIOD;
	animBodies[3].ascendingNode = PConst::EARTH_ASCENDING_NODE;
	animBodies[3].inclination = PConst::EARTH_INCLINATION;
	animBodies[3].axialTilt = PConst::EARTH_AXIAL_TILT;

	animBodies[4].radius = PConst::MARS_RADIUS;
	animBodies[4].orbitalPeriod = PConst::MARS_ORBITAL_PERIOD;
	animBodies[4].localOrbitalPeriod = PConst::MARS_LOCAL_ORBITAL_PERIOD;
	animBodies[4].ascendingNode = PConst::MARS_ASCENDING_NODE;
	animBodies[4].inclination = PConst::MARS_INCLINATION;
	animBodies[4].axialTilt = PConst::MARS_AXIAL_TILT;

	animBodies[5].radius = PConst::JUPITER_RADIUS;
	animBodies[5].orbitalPeriod = PConst::JUPITER_ORBITAL_PERIOD;
	animBodies[5].localOrbitalPeriod = PConst::JUPITER_LOCAL_ORBITAL_PERIOD;
	animBodies[5].ascendingNode = PConst::JUPITER_ASCENDING_NODE;
	animBodies[5].inclination = PConst::JUPITER_INCLINATION;
	animBodies[5].axialTilt = PConst::JUPITER_AXIAL_TILT;

	animBodies[6].radius = PConst::SATURN_RADIUS;
	animBodies[6].orbitalPeriod = PConst::SATURN_ORBITAL_PERIOD;
	animBodies[6].localOrbitalPeriod = PConst::SATURN_LOCAL_ORBITAL_PERIOD;
	animBodies[6].ascendingNode = PConst::SATURN_ASCENDING_NODE;
	animBodies[6].inclination = PConst::SATURN_INCLINATION;
	animBodies[6].axialTilt = PConst::SATURN_AXIAL_TILT;

	animBodies[7].radius = PConst::URANUS_RADIUS;
	animBodies[7].orbitalPeriod = PConst::URANUS_ORBITAL_PERIOD;
	animBodies[7].localOrbitalPeriod = PConst::URANUS_LOCAL_ORBITAL_PERIOD;
	animBodies[7].ascendingNode = PConst::URANUS_ASCENDING_NODE;
	animBodies[7].inclination = PConst::URANUS_INCLINATION;
	animBodies[7].axialTilt = PConst::URANUS_AXIAL_TILT;

	animBodies[8].radius = PConst::NEPTUNE_RADIUS;
	animBodies[8].orbitalPeriod = PConst::NEPTUNE_ORBITAL_PERIOD;
	animBodies[8].localOrbitalPeriod = PConst::NEPTUNE_LOCAL_ORBITAL_PERIOD;
	animBodies[8].ascendingNode = PConst::NEPTUNE_ASCENDING_NODE;
	animBodies[8].inclination = PConst::NEPTUNE_INCLINATION;
	animBodies[8].axialTilt = PConst::NEPTUNE_AXIAL_TILT;

	animBodies[9].radius = PConst::PLUTO_RADIUS;
	animBodies[9].orbitalPeriod = PConst::PLUTO_ORBITAL_PERIOD;
	animBodies[9].localOrbitalPeriod = PConst::PLUTO_LOCAL_ORBITAL_PERIOD;
	animBodies[9].ascendingNode = PConst::PLUTO_ASCENDING_NODE;
	animBodies[9].inclination = PConst::PLUTO_INCLINATION;
	animBodies[9].axialTilt = PConst::PLUTO_AXIAL_TILT;

	// bodies orbiting others

	animBodies[10].radius = PConst::MOON_RADIUS;
	animBodies[10].orbitalPeriod = PConst::MOON_ORBITAL_PERIOD;
	animBodies[10].localOrbitalPeriod = PConst::MOON_LOCAL_ORBITAL_PERIOD;
	animBodies[10].ascendingNode = 0;
	animBodies[10].inclination = PConst::MOON_INCLINATION;
	animBodies[10].axialTilt = PConst::MOON_AXIAL_TILT;

	return animBodies;
}

int PlanetMath::getPlanet(std::string name)
{
	return keyMap[name] - 1;
}

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

float PlanetMath::getMarginedRadius(float radiusA, float radiusB, float marginB, float modifierB)
{
	return 0.0f;
}

float PlanetMath::sumRecursiveParentInclination(std::vector<RenderedBody> rb, std::vector<BodyConst> bc, int targetIndex)
{
	// if not more parent then do this (base case)
	if (rb[targetIndex].orbitParentIdx == -1)
	{
		return bc[rb[targetIndex].bodyConstantIdx].inclination;
	}
	return bc[rb[targetIndex].bodyConstantIdx].inclination + sumRecursiveParentInclination(rb, bc, rb[targetIndex].orbitParentIdx);
}

//bool PlanetMath::findEqual(std::vector<float> values, float value)
//{
//	for (int j = 0; j < values.size(); j++)
//	{
//		if (values[j] == value)
//		{
//			return true;
//		}
//	}
//	return false;
//}


//std::vector<float> PlanetMath::batchGetRelativeValue(std::vector<float> as1, float bs1, float bs2)
//{
//	std::vector<float> out;
//	for (int i = 0; i < as1.size(); i++)
//	{
//		out.push_back(getRelativeValue(as1[i], bs1, bs2));
//	}
//	return out;
//}
