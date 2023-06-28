#include "timer.hpp"

#include <fmt/chrono.h>
#include <iomanip>
#include <memory>

namespace looper::time {

Timer::Timer()
   : m_deltaTime(milliseconds(0)),
     m_totalTime(milliseconds(0)),
     m_timeStamp(std::chrono::steady_clock::now())
{
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
   return GetMicroDeltaTime().count();
}

time::microseconds
Timer::GetMicroDeltaTime() const
{
   return m_deltaTime;
}

time::milliseconds
Timer::GetMsDeltaTime() const
{
   return std::chrono::duration_cast< milliseconds >(m_deltaTime);
}

float
Timer::GetTotalTime() const
{
   return static_cast< float >(m_totalTime.count());
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
   m_deltaTime = std::chrono::duration_cast< microseconds >(currentTimeStamp - m_timeStamp);
   m_timeStamp = currentTimeStamp;
   m_totalTime += std::chrono::duration_cast< milliseconds >(m_deltaTime);
}

time::milliseconds
Timer::ConvertToMs(seconds sec)
{
   return std::chrono::duration_cast< time::milliseconds >(sec);
}

std::string
Timer::GetCurrentTime()
{
   auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
   return fmt::format("{:%H:%M:%S}", fmt::localtime(time));
}

} // namespace looper::time
