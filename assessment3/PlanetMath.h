#pragma once

class PlanetConstants
{
public:

	// SUN
	static const float SUN_RADIUS;		// km
	static const float SUN_AXIAL_TILT;	// degree

	// EARTH
	static const float EARTH_RADIUS;				// radius in km
	static const float EARTH_AXIAL_TILT;			// north south axis tilt in degrees
	static const float EARTH_ORBITAL_PERIOD;		// earth days
	static const float EARTH_ECLIPTIC_INCLINATION;	// earth orbit tilt relative to sun degree (this is the main reference angle thus 0)
	static const float EARTH_DISTANCE_TO_SUN;		// million km (good to know but too large to be used)

	// MOON
	static const float MOON_RADIUS;						// radius in km
	static const float MOON_AXIAL_TILT;					// north south axis tilt in degrees
	static const float MOON_MOON_DAYS_ORBITAL_PERIOD;	// sidereal moon days
	static const float MOON_EARTH_DAYS_ORBITAL_PERIOD;	// sidereal earth days
	static const float MOON_ECLIPTIC_INCLINATION;		// orbit tilt relative to sun (probably, probably not)
	static const float MOON_DISTANCE_TO_EARTH;			// distance to earth in km
	static const float MOON_APOAPSIS;					// furthest distance to earth in km
	static const float MOON_PERIAPSIS;					// closest distance to earth in km

};

//class Body
//{
//public:
//	float radius;
//	float scale;
//	float scaleModifier;
//	float orbitRadius;
//	Body(float radius, float scale, float orbitRadius);
//};

class PlanetMath
{
public:
	float getRelativeScale(float radius1, float radius2, float scale2);
	float getRelativeScale(float radius1, float radius2, float scale2, float modifier);
	float getScaledRadiusDistance(float scale1, float scale2, float sceneObjectRadius, float modifier);
};
