#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>
#define DEG2RAD(n)	n*(M_PI/180)
#include <vector>



// animator that calculates positions and angles for simple circular or elipse shape orbit centered at origin
class OrbitAnimator
{
public:
	
	// Constructors

	OrbitAnimator(float orbitalDelay, float orbitalDays, float initialSpinAngle, 
		float initialOrbitAngle, float ovalRatio, float orbit_radius, float orbit_tilt);
	OrbitAnimator(float orbitalDelay, float orbitalDays, float ovalRatio, float orbit_radius, float orbit_tilt);
	OrbitAnimator() = default;

	// Essential Math input

	float getOrbitalDelay();
	void setOrbitalDelay(float seconds);

	float getOrbitalDays();
	void setOrbitalDays(float days);

	// Animation Values

	float getSpinAngle();
	void setSpinAngle(float degrees);

	float getOrbitAngle();
	void addOrbitAngle(float degress);
	void setOrbitAngle(float degrees);

	std::vector<float> getOrbitPosition();
	void setOrbitPosition(float x, float y, float z);

	// Cosmetics

	
	float getOvalRatio();
	void setOvalRatio(float ratio);

	void setOrbitRadius(float radius);
	float getOrbitRadius();

	void setOrbitTilt(float degrees);
	float getOrbitTilt();

	// Public Methods

	void animate(float current_ms_time, unsigned int spin_angle_precision, unsigned int orbit_angle_precision, bool force = false);
	void animate(std::vector<float> orbit_origin, float current_ms_time, unsigned int spin_angle_precision, unsigned int orbit_angle_precision, bool force = false);

private:

	// Private Attributes

	// configs for calculating following parameters
	float orbital_delay = 600.f;	// time in seconds to complete a full 360 degrees orbit
	float orbital_days = 1.f;		// total number of spins for every full orbit (animated object's day cycle, not earth's)

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
	void initDelays();

	// calculate new spin angle
	void updateSpin(float current_ms_time, unsigned int precision);
	// calculate new orbit angle and positions
	void updateOrbit(std::vector<float> orbit_origin, float current_ms_time, unsigned int precision, bool force = false);
	
};
