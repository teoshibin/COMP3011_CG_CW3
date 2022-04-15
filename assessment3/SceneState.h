#pragma once

class SceneState
{
public:
	SceneState(float pauseDelay = 0.25f);
	void pauseScene(double time);
	double getMsPlayTime(double time);
	bool getPause();
private:
	bool pause = false;
	float pauseDelay = 0.25f;
	double pauseTriggerTime = 0.f;
	double pauseStartTime = 0.f;
	double totalPausedTime = 0.f;
};
