#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>
#define DEG2RAD(n)	n*(M_PI/180)

struct Earth
{
	const float radius = 6371.f; // km
	const float axial_tilt = 23.4392811f; // degree
	const float orbital_period = 365.256363004f; // days
	const float orbital_speed_kmPs = 29.78f; // km/s
	const float ecliptic_inclination = 0.f; // degree
	const float distance_to_sun = 149.6e+6f; // million km
};

struct Sun
{
	const float radius = 696340.f; // km
	const float axial_tilt = 7.25f; // degree
};

struct SolarSystem
{
	struct Earth earth;
	struct Sun sun;
};


void earthOrbitAngle(float current_ms_time, float* previous_ms_time, float s_time_delay_per_angle, int precision, float radius, float major_axis_ratio, float* orbit_angle, float* posX, float* posY)
{
	// elapsed time in milliseconds
	float ms_duration = current_ms_time - *previous_ms_time;

	// angle precision to precision decimal points (higher precision smoother animation)
	float f_precision = 10 ^ (-precision);

	// actual delay required when stepping using precision
	float ms_time_delay = f_precision * s_time_delay_per_angle * 1000;

	// if elapsed time go pass the delay then increament spin angle by precision
	if (ms_duration > ms_time_delay)
	{
		// reset spin timer
		*previous_ms_time = current_ms_time;

		// step by precision but scaled, so that duration that go pass delay time can be added back into the angle
		*orbit_angle += (ms_duration / ms_time_delay) * f_precision;
		*orbit_angle = fmodf(*orbit_angle, 360.f);

		*posX = sin(DEG2RAD(*orbit_angle)) * radius * major_axis_ratio;
		*posY = cos(DEG2RAD(*orbit_angle)) * radius;
	}
}

void earthSpinAngle(float current_ms_time, float* previous_ms_time, float* spin_angle, float s_time_delay_per_angle, int precision)
{
	// elapsed time in milliseconds
	float ms_duration = current_ms_time - *previous_ms_time;
	
	// angle precision to precision decimal points (higher precision smoother animation)
	float f_precision = 10 ^ (-precision);

	// actual delay required when stepping using precision
	float ms_time_delay = f_precision * s_time_delay_per_angle * 1000;
	
	// if elapsed time go pass the delay then increament spin angle by precision
	if (ms_duration > ms_time_delay)
	{
		// reset spin timer
		*previous_ms_time = current_ms_time;

		// step by precision but scaled, so that duration that go pass delay time can be added back into the angle
		*spin_angle += (ms_duration / ms_time_delay) * f_precision;
		
		*spin_angle = fmodf(*spin_angle, 360.f);
	}
}