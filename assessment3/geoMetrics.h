#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>
#define DEG2RAD(n)	n*(M_PI/180)
#include <vector>

class UniversalConstants
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

//STRUCT EARTH
//{
//	CONST FLOAT RADIUS = 6371.F; // RADIUS IN KM
//	CONST FLOAT AXIAL_TILT = 23.4392811F; // NORTH SOUTH AXIS TILT IN DEGREES
//	CONST FLOAT ORBITAL_PERIOD = 365.256363004F; // EARTH DAYS
//	CONST FLOAT ECLIPTIC_INCLINATION = 0.F; // EARTH ORBIT TILT RELATIVE TO SUN DEGREE (THIS IS THE MAIN REFERENCE ANGLE THUS 0)
//	CONST FLOAT DISTANCE_TO_SUN = 149.6E+6F; // MILLION KM (GOOD TO KNOW BUT TOO LARGE TO BE USED)
//};
//
//STRUCT MOON
//{
//	CONST FLOAT RADIUS = 1737.4; // RADIUS IN KM
//	CONST FLOAT AXIAL_TILT = 1.5424F; // NORTH SOUTH AXIS TILT IN DEGREES
//	CONST FLOAT MOON_DAYS_ORBITAL_PERIOD = 1.F; // SIDEREAL MOON DAYS
//	CONST FLOAT EARTH_DAYS_ORBITAL_PERIOD = 27.321661F; // SIDEREAL EARTH DAYS
//	CONST FLOAT ECLIPTIC_INCLINATION = 5.145F; // ORBIT TILT RELATIVE TO SUN (PROBABLY, PROBABLY NOT)
//	CONST FLOAT DISTANCE_TO_EARTH = 384400.F; // DISTANCE TO EARTH IN KM
//	CONST FLOAT APOAPSIS = 405400.F; // FURTHEST DISTANCE TO EARTH IN KM
//	CONST FLOAT PERIAPSIS = 362600.F; // CLOSEST DISTANCE TO EARTH IN KM
//};
//
//STRUCT SUN
//{
//	CONST FLOAT RADIUS = 696340.F; // KM
//	CONST FLOAT AXIAL_TILT = 7.25F; // DEGREE
//};
//
//STRUCT SOLARSYSTEM
//{
//	STRUCT EARTH EARTH;
//	STRUCT SUN SUN;
//	STRUCT MOON MOON;
//};
//struct Earth
//{
//	const float radius = 6371.f; // radius in km
//	const float axial_tilt = 23.4392811f; // north south axis tilt in degrees
//	const float orbital_period = 365.256363004f; // earth days
//	const float ecliptic_inclination = 0.f; // earth orbit tilt relative to sun degree (this is the main reference angle thus 0)
//	const float distance_to_sun = 149.6e+6f; // million km (good to know but too large to be used)
//};
//
//struct Moon
//{
//	const float radius = 1737.4; // radius in km
//	const float axial_tilt = 1.5424f; // north south axis tilt in degrees
//	const float moon_days_orbital_period = 1.f; // sidereal moon days
//	const float earth_days_orbital_period = 27.321661f; // sidereal earth days
//	const float ecliptic_inclination = 5.145f; // orbit tilt relative to sun (probably, probably not)
//	const float distance_to_earth = 384400.f; // distance to earth in km
//	const float apoapsis = 405400.f; // furthest distance to earth in km
//	const float periapsis = 362600.f; // closest distance to earth in km
//};
//
//struct Sun
//{
//	const float radius = 696340.f; // km
//	const float axial_tilt = 7.25f; // degree
//};
//
//struct SolarSystem
//{
//	struct Earth earth;
//	struct Sun sun;
//	struct Moon moon;
//};

class SphereAnimator
{
public:
	
	// Constructors

	SphereAnimator(float orbitalDelay, float orbitalDays, float initialSpinAngle, 
		float initialOrbitAngle, float ovalRatio, float orbit_radius, float orbit_tilt);
	SphereAnimator(float orbitalDelay, float orbitalDays, float ovalRatio, float orbit_radius, float orbit_tilt);
	SphereAnimator() = default;

	// Essential Math input

	// get time duration of completing full orbit of the sun
	float getOrbitalDelay();
	// set time duration in seconds for completing full orbit of the sun
	void setOrbitalDelay(float seconds);

	// get total of days for one full orbit
	float getOrbitalDays();
	// set total of days for one full orbit
	void setOrbitalDays(float days);

	// Animation Values

	// get current day cycle angle
	float getSpinAngle();
	// set current day cycle angle
	void setSpinAngle(float degrees);

	// get current orbit degrees angle that is used for orbit position calculation
	float getOrbitAngle();
	// set current orbit degrees angle that is used for orbit position calculation
	void setOrbitAngle(float degrees);

	// get calculated orbit position
	std::vector<float> getOrbitPosition();
	// set orbit position
	void setOrbitPosition(float x, float y, float z);

	// Cosmetics

	// get orbit ratio for major radius axis
	float getOvalRatio();
	// set orbit ratio for major radius axis
	void setOvalRatio(float ratio);

	// set distance of edge to center
	void setOrbitRadius(float radius);
	// get distance of edge to center
	float getOrbitRadius();

	// set distance of edge to center
	void setOrbitTilt(float degrees);
	// get distance of edge to center
	float getOrbitTilt();

	// Public Methods

	// animate
	void animate(float current_ms_time, unsigned int spin_angle_precision, unsigned int orbit_angle_precision);

private:

	// Private Attributes

	// configs for calculating following parameters
	float orbital_delay = 600.f;			// time in seconds to complete a full 360 degrees orbit
	float orbital_days = 1.f;				// total number of spins for every full orbit (animated object's day cycle, not earth's)

	// delays
	float delay_per_orbit_angle = orbital_delay / 360;	// delay for every orbit angle change in seconds
	float delay_per_day = orbital_delay / orbital_days;	// delay for a day in seconds (intermediate calculation)
	float delay_per_spin_angle = delay_per_day / 360;	// delay for every spin angle change in seconds

	// day cycle
	float spin_angle = 0.f;					// self rotation angle [1,360)
	float previous_spin_timestamp = 0.f;	// store previous spin angle change timestamp in seconds

	// year cycle
	float orbit_angle = 0.f;				// orbiting angle [1,360)
	float previous_orbit_timestamp = 0.f;		// store previous orbit angle chnage timestamp in seconds
	std::vector<float> orbit_position{ 0.f, 0.f, 0.f }; // x,y,z (width, height, depth)

	// cosmestics
	float oval_ratio = 1.f;					// scale of major axis over minor axis
	float orbit_radius = 1.f;				// radius to the center of the orbit
	float orbit_tilt = 0.f;					// orbit tilt on x axis

	// Private Methods
	
	// step count an angle with certain delay and precision
	bool stepAngle(float current_ms_time, float* previous_timestamp, float s_delay_per_angle, unsigned int precision, float* angle);
	// update delays calculation whenever hyperparam is changed
	void updateDelays();

	// calculate new spin angle
	void updateSpin(float current_ms_time, unsigned int precision);
	// calculate new orbit angle and positions
	void updateOrbit(float current_ms_time, unsigned int precision);
	
};
