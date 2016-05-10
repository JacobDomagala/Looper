#pragma once
#include"Common.h"

class Timer {
	LARGE_INTEGER frequency;
	LARGE_INTEGER counter;
	double deltaTime;
	double globalTime;

public:
	Timer();

	void ToggleTimer();

	float GetDeltaTime()
	{
		return static_cast<float>(deltaTime);
	}
	float GetGlobalTime()
	{
		return static_cast<float>(globalTime);
	}
};


