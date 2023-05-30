#include "time_step.hpp"

#include <chrono>
#include <fmt/format.h>

namespace looper::time {

TimeStep::TimeStep(milliseconds time) : m_time(std::chrono::duration_cast< milliseconds >(time))
{
}

TimeStep::TimeStep(microseconds time) : m_time(time)
{
}

std::string
TimeStep::ToString() const
{
   return fmt::format("{}ms", m_time.count());
}

seconds
TimeStep::GetSeconds() const
{
   return std::chrono::duration_cast< seconds >(m_time);
}

milliseconds
TimeStep::GetMilliseconds() const
{
   return std::chrono::duration_cast< milliseconds >(m_time);
}

microseconds
TimeStep::GetMicroseconds() const
{
   return m_time;
}

} // namespace looper::time
