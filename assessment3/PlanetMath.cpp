#include "PlanetMath.h"

// Constants

// SUN
const float PlanetConstants::SUN_RADIUS = 696340.f;	// km
const float PlanetConstants::SUN_AXIAL_TILT = 7.25f;	// degree

// EARTH
const float PlanetConstants::EARTH_RADIUS = 6371.f;					// radius in km
const float PlanetConstants::EARTH_AXIAL_TILT = 23.4392811f;			// north south axis tilt in degrees
const float PlanetConstants::EARTH_ORBITAL_PERIOD = 365.256363004f;	// earth days
const float PlanetConstants::EARTH_ECLIPTIC_INCLINATION = 0.f;		// earth orbit tilt relative to sun degree (this is the main reference angle thus 0)
const float PlanetConstants::EARTH_DISTANCE_TO_SUN = 149.6e+6f;		// million km (good to know but too large to be used)

// MOON
const float PlanetConstants::MOON_RADIUS = 1737.4f;							// radius in km
const float PlanetConstants::MOON_AXIAL_TILT = 1.5424f;						// north south axis tilt in degrees
const float PlanetConstants::MOON_MOON_DAYS_ORBITAL_PERIOD = 1.f;			// sidereal moon days
const float PlanetConstants::MOON_EARTH_DAYS_ORBITAL_PERIOD = 27.321661f;	// sidereal earth days
const float PlanetConstants::MOON_ECLIPTIC_INCLINATION = 5.145f;				// orbit tilt relative to sun (probably, probably not)
const float PlanetConstants::MOON_DISTANCE_TO_EARTH = 384400.f;				// distance to earth in km
const float PlanetConstants::MOON_APOAPSIS = 405400.f;						// furthest distance to earth in km
const float PlanetConstants::MOON_PERIAPSIS = 362600.f;						// closest distance to earth in km

/// <summary>
/// calculate object 1 scale using object 1 radius and object 2 radius and scale
/// </summary>
/// <param name="radius1">object 1 radius</param>
/// <param name="radius2">object 2 radius</param>
/// <param name="scale2">object 2 scale</param>
/// <returns>object 1 scale</returns>
float PlanetMath::getRelativeScale(float radius1, float radius2, float scale2)
{
	return radius1 / radius2 * scale2;
}

/// <summary>
/// calculate object 1 scale using object 1 radius and object 2 radius and scale then apply custom modifier scale
/// </summary>
/// <param name="radius1">object 1 radius</param>
/// <param name="radius2">object 2 radius</param>
/// <param name="scale2">object 2 scale</param>
/// <returns>modified object 1 scale</returns>
float PlanetMath::getRelativeScale(float radius1, float radius2, float scale2, float modifier)
{
	return radius1 / radius2 * scale2 * modifier;
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

