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
