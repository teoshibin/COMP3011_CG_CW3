#include "DelayTrigger.h"

bool DelayTrigger::toggle(float time, bool force)
{
	if (((time - previousTriggerTime) > delay) || force)
	{
		previousTriggerTime = time;
		value = !value;
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
