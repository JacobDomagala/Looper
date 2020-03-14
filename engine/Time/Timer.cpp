#include "Timer.hpp"
#include <memory>

Timer::Timer()
{
   m_deltaTime = std::chrono::milliseconds(0);
   m_timeStamp = std::chrono::steady_clock::now();
}

void
Timer::Start()
{
   // m_startPoint = std::chrono::steady_clock::now();
}

void
Timer::Stop()
{
   // auto tp = std::chrono::steady_clock::now();
}