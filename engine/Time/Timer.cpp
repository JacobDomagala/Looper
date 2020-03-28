#include "Timer.hpp"
#include <memory>

Timer::Timer()
{
   m_deltaTime = milliseconds(0);
   m_timeStamp = std::chrono::steady_clock::now();
   m_totalTime = milliseconds(0);
}
