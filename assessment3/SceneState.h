#pragma once

class SceneState
{
public:
	SceneState(float pauseDelay = 0.25f);
	// pause function with latch system to prevent multiple triggers
	void pauseScene(double time, bool force = false);
	double getMsPlayTime(double ms);
	bool getPause();
	void addSPlayTime(double s);
private:
	bool pause = false;
	bool justLoaded = true;
	float pauseDelay = 0.25f;
	double pauseTriggerTime = 0.f;
	double pauseStartTime = 0.f;
	double totalPausedTime = 0.f;
};
