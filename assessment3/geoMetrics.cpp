
#include "geoMetrics.h"

// Contants

// SUN
const float UniversalConstants::SUN_RADIUS = 696340.f;	// km
const float UniversalConstants::SUN_AXIAL_TILT = 7.25f;	// degree

// EARTH
const float UniversalConstants::EARTH_RADIUS = 6371.f;					// radius in km
const float UniversalConstants::EARTH_AXIAL_TILT = 23.4392811f;			// north south axis tilt in degrees
const float UniversalConstants::EARTH_ORBITAL_PERIOD = 365.256363004f;	// earth days
const float UniversalConstants::EARTH_ECLIPTIC_INCLINATION = 0.f;		// earth orbit tilt relative to sun degree (this is the main reference angle thus 0)
const float UniversalConstants::EARTH_DISTANCE_TO_SUN = 149.6e+6f;		// million km (good to know but too large to be used)

// MOON
const float UniversalConstants::MOON_RADIUS = 1737.4f;							// radius in km
const float UniversalConstants::MOON_AXIAL_TILT = 1.5424f;						// north south axis tilt in degrees
const float UniversalConstants::MOON_MOON_DAYS_ORBITAL_PERIOD = 1.f;			// sidereal moon days
const float UniversalConstants::MOON_EARTH_DAYS_ORBITAL_PERIOD = 27.321661f;	// sidereal earth days
const float UniversalConstants::MOON_ECLIPTIC_INCLINATION = 5.145f;				// orbit tilt relative to sun (probably, probably not)
const float UniversalConstants::MOON_DISTANCE_TO_EARTH = 384400.f;				// distance to earth in km
const float UniversalConstants::MOON_APOAPSIS = 405400.f;						// furthest distance to earth in km
const float UniversalConstants::MOON_PERIAPSIS = 362600.f;						// closest distance to earth in km

// Crude Orbit Calculations

OrbitAnimator::OrbitAnimator(float orbitalDelay, float orbitalDays, float initialSpinAngle, float initialOrbitAngle, float ovalRatio, float orbit_radius, float orbit_tilt)
{
	OrbitAnimator::setOrbitalDelay(orbitalDelay);
	OrbitAnimator::setOrbitalDays(orbitalDays);
	OrbitAnimator::setSpinAngle(initialSpinAngle);
	OrbitAnimator::setOrbitAngle(initialOrbitAngle);
	OrbitAnimator::setOvalRatio(ovalRatio);
	OrbitAnimator::setOrbitRadius(orbit_radius);
	OrbitAnimator::setOrbitTilt(orbit_tilt);
}

OrbitAnimator::OrbitAnimator(float orbitalDelay, float orbitalDays, float ovalRatio, float orbit_radius, float orbit_tilt)
{
	OrbitAnimator::setOrbitalDelay(orbitalDelay);
	OrbitAnimator::setOrbitalDays(orbitalDays);
	OrbitAnimator::setOvalRatio(ovalRatio);
	OrbitAnimator::setOrbitRadius(orbit_radius);
	OrbitAnimator::setOrbitTilt(orbit_tilt);
}

void OrbitAnimator::setOrbitalDays(float days) 
{ 
	orbital_days = days; 
	updateDelays();
}
float OrbitAnimator::getOrbitalDays() { return orbital_days; }

void OrbitAnimator::setOrbitalDelay(float seconds)
{ 
	orbital_delay = seconds; 
	updateDelays();
}
float OrbitAnimator::getOrbitalDelay() { return orbital_delay; }

void OrbitAnimator::setOrbitAngle(float degrees) { orbit_angle = degrees; }
float OrbitAnimator::getOrbitAngle() { return orbit_angle; }

void OrbitAnimator::setSpinAngle(float degrees) { spin_angle = degrees; }
float OrbitAnimator::getSpinAngle() { return spin_angle; }

void OrbitAnimator::setOvalRatio(float ratio) { oval_ratio = ratio; }
float OrbitAnimator::getOvalRatio() { return oval_ratio; }

void OrbitAnimator::setOrbitRadius(float radius) { orbit_radius = radius; }
float OrbitAnimator::getOrbitRadius() { return orbit_radius; }

void OrbitAnimator::setOrbitTilt(float degrees) { orbit_tilt = degrees; }
float OrbitAnimator::getOrbitTilt() { return orbit_tilt; }

void OrbitAnimator::setOrbitPosition(float x, float y, float z)
{
	orbit_position[0] = x;
	orbit_position[1] = y;
	orbit_position[2] = z;
}
std::vector<float> OrbitAnimator::getOrbitPosition() { return orbit_position; }

void OrbitAnimator::animate(float current_ms_time, unsigned int spin_angle_precision, unsigned int orbit_angle_precision)
{
	std::vector<float> orbit_origin{ 0.f,0.f,0.f };
	OrbitAnimator::updateSpin(current_ms_time, spin_angle_precision);
	OrbitAnimator::updateOrbit(orbit_origin, current_ms_time, orbit_angle_precision);
}

void OrbitAnimator::animate(std::vector<float> orbit_origin, float current_ms_time, unsigned int spin_angle_precision, unsigned int orbit_angle_precision)
{
	OrbitAnimator::updateSpin(current_ms_time, spin_angle_precision);
	OrbitAnimator::updateOrbit(orbit_origin, current_ms_time, orbit_angle_precision);
}

void OrbitAnimator::updateDelays()
{
	delay_per_orbit_angle = orbital_delay / 360;	// delay for every orbit angle change in seconds
	delay_per_day = orbital_delay / orbital_days;	// delay for a day in seconds (intermediate calculation)
	delay_per_spin_angle = delay_per_day / 360;	    // delay for every spin angle change in seconds
}

void OrbitAnimator::updateSpin(float current_ms_time, unsigned int precision) 
{
	stepAngle(current_ms_time, &previous_spin_timestamp, delay_per_spin_angle, precision, &spin_angle);
}

void OrbitAnimator::updateOrbit(std::vector<float> orbit_origin, float current_ms_time, unsigned int precision)
{
	if (stepAngle(current_ms_time, &previous_orbit_timestamp, delay_per_orbit_angle, precision, &orbit_angle))
	{
		float x = (sin(DEG2RAD(orbit_angle)) * orbit_radius * oval_ratio);
		float y = (tan(DEG2RAD(orbit_tilt)) * x);
		float z = (cos(DEG2RAD(orbit_angle)) * orbit_radius);
		setOrbitPosition(x + orbit_origin[0], y + orbit_origin[1], z + orbit_origin[2]);
	}
}

bool OrbitAnimator::stepAngle(float current_ms_time, float* previous_timestamp, float s_delay_per_angle, unsigned int precision, float* angle)
{
	// elapsed time in milliseconds
	float ms_duration = current_ms_time - *previous_timestamp;

	// angle precision to precision decimal points (higher precision smoother animation)
	float f_precision = powf(10, -(int)precision);

	// actual delay required when stepping using precision
	float ms_time_delay = f_precision * s_delay_per_angle * 1000;

	// if elapsed time go pass the delay then increament spin angle by precision
	if (ms_duration > ms_time_delay)
	{
		// reset spin timer
		*previous_timestamp = current_ms_time;

		// step by precision but scaled, so that duration that go pass delay time can be added back into the angle
		*angle += (ms_duration / ms_time_delay) * f_precision;

		// make sure it doesn't go out of bound
		*angle = fmodf(*angle, 360.f);

		return true;
	}

	return false;
}