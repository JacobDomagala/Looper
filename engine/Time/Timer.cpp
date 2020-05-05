#include "Timer.hpp"
#include <memory>

namespace dgame {

Timer::Timer()
{
   m_deltaTime = milliseconds(0);
   m_timeStamp = std::chrono::steady_clock::now();
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

} // namespace dgame