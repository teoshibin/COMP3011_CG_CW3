#pragma once
class DelayTrigger
{
public:
	DelayTrigger();
	DelayTrigger(float delay);
	bool toggle(float time, bool force = false);
	void setValue(bool value);
	bool getValue();
	void setDelay(float delay);
	float getDiffRatio();
private:
	bool value = false;
	float delay = 0.25f;
	float previousTriggerTime = 0.f;
	float lastDiffRatio = 0.f;
};

