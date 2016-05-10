#include "Timer.h"

Timer::Timer()
{
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&counter);
	globalTime = 0;
	deltaTime = 0;
}
void Timer::ToggleTimer()
{
	LARGE_INTEGER tmp;
	QueryPerformanceCounter(&tmp);
	__int64 deltaCount = tmp.QuadPart - counter.QuadPart;
	deltaTime = deltaCount / static_cast<double>(frequency.QuadPart);

	globalTime += deltaTime;
	counter = tmp;
}

