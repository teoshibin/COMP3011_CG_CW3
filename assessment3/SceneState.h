#pragma once

#include <iostream>
#include "DelayTrigger.h"

class SceneState
{
public:
	void pauseScene(double time, bool force = false);
	double getMsPlayTime(double ms);
	bool getPause();
	void addSPlayTime(double s);
	bool getJustStarted();
	void setJustStarted(bool value);
	bool getCanUpdateAnimation();
private:
	DelayTrigger pausing;
	bool justStarted = true;
	double pauseStartTime = 0.f;
	double totalPausedTime = 0.f;
};
