#pragma once
class DelayTrigger
{
public:
	bool trigger(float time, bool force = false);
	void setValue(bool value);
	bool getValue();
	void setDelay(float delay);
private:
	bool value = false;
	float delay = 0.25f;
	float previousTriggerTime = 0.f;
};

