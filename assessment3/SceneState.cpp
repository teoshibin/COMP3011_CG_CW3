#include "SceneState.h"

SceneState::SceneState(float newPauseDelay)
{
	pauseDelay = newPauseDelay;
}

void SceneState::pauseScene(double time)
{
	double pausedTime = time - pauseTriggerTime;
	if (pausedTime > pauseDelay)
	{
		pauseTriggerTime = time;
		pause = !pause;
		if (pause)
		{
			pauseStartTime = time;
		}
		else
		{
			totalPausedTime += time - pauseStartTime;
		}
	}
}

double SceneState::getMsPlayTime(double time)
{
	return (time - totalPausedTime) * 1000;
}

bool SceneState::getPause()
{
	return pause;
}