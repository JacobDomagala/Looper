#include "scoped_timer.hpp"
#include "Logger/logger.hpp"

namespace looper::time {

ScopedTimer::ScopedTimer(std::string&& logMsg) : m_logMsg(std::move(logMsg))
{
   m_timer.Start();
}

ScopedTimer::~ScopedTimer()
{
   Logger::Debug("{} took {}", m_logMsg, m_timer.Stop().ToString());
}

} // namespace shady::time
