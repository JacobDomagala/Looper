#pragma once
#include"Common.h"

class Timer {
	LARGE_INTEGER frequency;
	LARGE_INTEGER counter;
	double deltaTime;
	double globalTime;
	double accumulator;
public:
	Timer();

	void ToggleTimer();

	void Accumulate()
	{
		accumulator += deltaTime;
	}
	void ToggleAndAccumulate()
	{
		ToggleTimer();
		accumulator += deltaTime;
	}
	void ResetAccumulator()
	{
		accumulator = 0;
	}
	float GetAccumulator() const 
	{
		return static_cast<float>(accumulator);
	}
	float GetDeltaTime() const
	{
		return static_cast<float>(deltaTime);
	}
	float GetGlobalTime() const
	{
		return static_cast<float>(globalTime);
	}
};


