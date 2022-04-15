#pragma once

#ifndef MYLIB_CONSTANTS_H
#define MYLIB_CONSTANTS_H

namespace PConst
{
	// SUN
	const float SUN_RADIUS = 696340.f;	// km
	const float SUN_AXIAL_TILT = 7.25f;	// degree

	// MERCURY
	//const float MERCURY_LENGTH_OF_DAY;

	// EARTH
	const float EARTH_RADIUS = 6371.f;					// radius in km
	const float EARTH_AXIAL_TILT = 23.4392811f;			// north south axis tilt in degrees
	const float EARTH_ORBITAL_PERIOD = 365.256363004f;	// earth days
	const float EARTH_ECLIPTIC_INCLINATION = 0.f;		// earth orbit tilt relative to sun degree (this is the main reference angle thus 0)
	//const float EARTH_DISTANCE_TO_SUN = 149.6e+6f;	// million km (good to know but too large to be used)

	// MOON
	const float MOON_RADIUS = 1737.4f;						// radius in km
	const float MOON_AXIAL_TILT = 1.5424f;					// north south axis tilt in degrees
	const float MOON_SELF_DAY_ORBITAL_PERIOD = 1.f;			// sidereal moon days
	const float MOON_EARTH_DAY_ORBITAL_PERIOD = 27.321661f;	// sidereal earth days
	const float MOON_ECLIPTIC_INCLINATION = 5.145f;			// orbit tilt relative to sun (probably, probably not)
	//const float MOON_LENGTH_OF_DAY = 27.321661f;
	//const float MOON_DISTANCE_TO_EARTH = 384400.f;		// distance to earth in km
	//const float MOON_APOAPSIS = 405400.f;					// furthest distance to earth in km
	//const float MOON_PERIAPSIS = 362600.f;				// closest distance to earth in km

}
#endif

class PlanetMath
{
public:
	float getRelativeValue(float a1, float b1, float b2);
	float getRelativeValue(float a1, float b1, float b2, float r);
	float getScaledRadiusDistance(float scale1, float scale2, float sceneObjectRadius, float modifier);
	float earthDayToLocalDay(float earthDays, float localDayLength);
};
