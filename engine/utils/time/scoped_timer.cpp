#include "scoped_timer.hpp"
#include "logger/logger.hpp"

namespace looper::time {

ScopedTimer::ScopedTimer(std::string&& logMsg) : m_logMsg(std::move(logMsg))
{
   m_timer.Start();
}

//NOLINTNEXTLINE
ScopedTimer::~ScopedTimer()
{
   Logger::Debug("{} took {}ms", m_logMsg, m_timer.Stop().GetMilliseconds().count());
}

} // namespace shady::time
