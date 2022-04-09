
#include "geoMetrics.h"

SphereAnimator::SphereAnimator(float orbitalDelay, float orbitalDays, float initialSpinAngle, float initialOrbitAngle, float ovalRatio, float orbit_radius, float orbit_tilt)
{
	SphereAnimator::setOrbitalDelay(orbitalDelay);
	SphereAnimator::setOrbitalDays(orbitalDays);
	SphereAnimator::setSpinAngle(initialSpinAngle);
	SphereAnimator::setOrbitAngle(initialOrbitAngle);
	SphereAnimator::setOvalRatio(ovalRatio);
	SphereAnimator::setOrbitRadius(orbit_radius);
	SphereAnimator::setOrbitTilt(orbit_tilt);
}

SphereAnimator::SphereAnimator(float orbitalDelay, float orbitalDays, float ovalRatio, float orbit_radius, float orbit_tilt)
{
	SphereAnimator::setOrbitalDelay(orbitalDelay);
	SphereAnimator::setOrbitalDays(orbitalDays);
	SphereAnimator::setOvalRatio(ovalRatio);
	SphereAnimator::setOrbitRadius(orbit_radius);
	SphereAnimator::setOrbitTilt(orbit_tilt);
}

void SphereAnimator::setOrbitalDays(float days) 
{ 
	orbital_days = days; 
	updateDelays();
}
float SphereAnimator::getOrbitalDays() { return orbital_days; }

void SphereAnimator::setOrbitalDelay(float seconds)
{ 
	orbital_delay = seconds; 
	updateDelays();
}
float SphereAnimator::getOrbitalDelay() { return orbital_delay; }

void SphereAnimator::setOrbitAngle(float degrees) { orbit_angle = degrees; }
float SphereAnimator::getOrbitAngle() { return orbit_angle; }

void SphereAnimator::setSpinAngle(float degrees) { spin_angle = degrees; }
float SphereAnimator::getSpinAngle() { return spin_angle; }

void SphereAnimator::setOvalRatio(float ratio) { oval_ratio = ratio; }
float SphereAnimator::getOvalRatio() { return oval_ratio; }

void SphereAnimator::setOrbitRadius(float radius) { orbit_radius = radius; }
float SphereAnimator::getOrbitRadius() { return orbit_radius; }

void SphereAnimator::setOrbitTilt(float degrees) { orbit_tilt = degrees; }
float SphereAnimator::getOrbitTilt() { return orbit_tilt; }

void SphereAnimator::setOrbitPosition(float x, float y, float z)
{
	orbit_position[0] = x;
	orbit_position[1] = y;
	orbit_position[2] = z;
}
std::vector<float> SphereAnimator::getOrbitPosition() { return orbit_position; }

void SphereAnimator::animate(float current_ms_time, unsigned int spin_angle_precision, unsigned int orbit_angle_precision)
{
	SphereAnimator::updateSpin(current_ms_time, spin_angle_precision);
	SphereAnimator::updateOrbit(current_ms_time, orbit_angle_precision);
}

void SphereAnimator::updateDelays()
{
	delay_per_orbit_angle = orbital_delay / 360;	// delay for every orbit angle change in seconds
	delay_per_day = orbital_delay / orbital_days;	// delay for a day in seconds (intermediate calculation)
	delay_per_spin_angle = delay_per_day / 360;	    // delay for every spin angle change in seconds
}

void SphereAnimator::updateSpin(float current_ms_time, unsigned int precision) 
{
	stepAngle(current_ms_time, &previous_spin_timestamp, delay_per_spin_angle, precision, &spin_angle);
}

void SphereAnimator::updateOrbit(float current_ms_time, unsigned int precision)
{
	bool stepped = stepAngle(current_ms_time, &previous_orbit_timestamp, delay_per_orbit_angle, precision, &orbit_angle);
	if (stepped)
	{
		float x = sin(DEG2RAD(orbit_angle)) * orbit_radius * oval_ratio;
		float y = tan(DEG2RAD(orbit_tilt)) * x;
		float z = cos(DEG2RAD(orbit_angle)) * orbit_radius;
		setOrbitPosition(x, y, z);
	}
}

bool SphereAnimator::stepAngle(float current_ms_time, float* previous_timestamp, float s_delay_per_angle, unsigned int precision, float* angle)
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