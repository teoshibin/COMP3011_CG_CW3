#include "DelayTrigger.h"

DelayTrigger::DelayTrigger()
{
}

DelayTrigger::DelayTrigger(float delay)
{
	setDelay(delay);
}

bool DelayTrigger::toggle(float time, bool force)
{
	float diff = time - previousTriggerTime;
	if ((diff > delay) || force)
	{
		previousTriggerTime = time;
		value = !value;
		lastDiffRatio = diff/delay;
		return true;
	}
	return false;
}

void DelayTrigger::setValue(bool newValue)
{
	value = newValue;
}

bool DelayTrigger::getValue()
{
	return value;
}

void DelayTrigger::setDelay(float newDelay)
{
	delay = newDelay;
}

float DelayTrigger::getDiffRatio()
{
	return lastDiffRatio > 10? 1: lastDiffRatio;
}
