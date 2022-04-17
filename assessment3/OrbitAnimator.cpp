
#include "OrbitAnimator.h"

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
	initDelays();
}
float OrbitAnimator::getOrbitalDays() { return orbital_days; }

void OrbitAnimator::setOrbitalDelay(float seconds)
{ 
	orbital_delay = seconds; 
	initDelays();
}
float OrbitAnimator::getOrbitalDelay() { return orbital_delay; }

void OrbitAnimator::setOrbitAngle(float degrees) { orbit_angle = degrees; }
float OrbitAnimator::getOrbitAngle() { return orbit_angle; }

void OrbitAnimator::addOrbitAngle(float degress)
{
	orbit_angle += degress;
}

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

void OrbitAnimator::animate(float current_ms_time, unsigned int spin_angle_precision, unsigned int orbit_angle_precision, bool force)
{
	std::vector<float> orbit_origin{ 0.f,0.f,0.f };
	OrbitAnimator::updateSpin(current_ms_time, spin_angle_precision);
	OrbitAnimator::updateOrbit(orbit_origin, current_ms_time, orbit_angle_precision, force);
}

void OrbitAnimator::animate(std::vector<float> orbit_origin, float current_ms_time, unsigned int spin_angle_precision, unsigned int orbit_angle_precision, bool force)
{
	OrbitAnimator::updateSpin(current_ms_time, spin_angle_precision);
	OrbitAnimator::updateOrbit(orbit_origin, current_ms_time, orbit_angle_precision, force);
}

void OrbitAnimator::initDelays()
{
	delay_per_orbit_angle = orbital_delay / 360;	// delay for every orbit angle change in seconds
	delay_per_day = orbital_delay / orbital_days;	// delay for a day in seconds (intermediate calculation)
	delay_per_spin_angle = delay_per_day / 360;	    // delay for every spin angle change in seconds
}

void OrbitAnimator::updateSpin(float current_ms_time, unsigned int precision) 
{
	stepAngle(current_ms_time, &previous_spin_timestamp, delay_per_spin_angle, precision, &spin_angle);
}

void OrbitAnimator::updateOrbit(std::vector<float> orbit_origin, float current_ms_time, unsigned int precision, bool force)
{
	if (stepAngle(current_ms_time, &previous_orbit_timestamp, delay_per_orbit_angle, precision, &orbit_angle) || force)
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