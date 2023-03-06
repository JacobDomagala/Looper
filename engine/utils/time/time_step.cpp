#include "time_step.hpp"

#include <chrono>
#include <fmt/format.h>

namespace looper::time {

TimeStep::TimeStep(milliseconds time) : m_time(time)
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
   return m_time;
}

}
