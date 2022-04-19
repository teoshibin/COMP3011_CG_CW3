#include "SceneState.h"

void SceneState::pauseScene(double time, bool force)
{
	if (pausing.trigger(time, force))
	{
		if (pausing.getValue())
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
	return pausing.getValue();
}

void SceneState::addSPlayTime(double s)
{
	totalPausedTime += s;
}

bool SceneState::getJustStarted()
{
	return justStarted;
}

void SceneState::setJustStarted(bool value)
{
	justStarted = value;
}

bool SceneState::getCanUpdateAnimation()
{
	return !pausing.getValue() || justStarted;
}
