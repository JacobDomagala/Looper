#include "Timer.hpp"

#include <ctime>
#include <iomanip>
#include <memory>
#include <sstream>

namespace dgame {

Timer::Timer()
{
   m_deltaTime = milliseconds(0);
   m_timeStamp = std::chrono::steady_clock::now();
   m_totalTime = milliseconds(0);
}

void
Timer::PauseAllTimers()
{
   m_timersPaused = true;
}

void
Timer::ResumeAllTimers()
{
   m_timersPaused = false;
}

bool
Timer::AreTimersRunning()
{
   return !m_timersPaused;
}

float
Timer::GetFloatDeltaTime() const
{
   return m_deltaTime.count();
}

Timer::milliseconds
Timer::GetMsDeltaTime() const
{
   return m_deltaTime;
}

float
Timer::GetTotalTime() const
{
   return std::chrono::duration_cast< milliseconds >(m_totalTime).count() / 1.0f;
}

void
Timer::ResetTotalTime()
{
   m_totalTime = milliseconds(0);
}

void
Timer::ToggleTimer()
{
   auto currentTimeStamp = std::chrono::steady_clock::now();
   m_deltaTime = std::chrono::duration_cast< milliseconds >(currentTimeStamp - m_timeStamp);
   m_timeStamp = currentTimeStamp;
   m_totalTime += m_deltaTime;
}

Timer::milliseconds
Timer::ConvertToMs(seconds sec)
{
   return std::chrono::duration_cast< Timer::milliseconds >(sec);
}

std::string
Timer::GetCurrentTime()
{
   auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
   std::stringstream ss;
   ss << std::put_time(std::localtime(&time), "%T");

   return ss.str();
}

} // namespace dgame