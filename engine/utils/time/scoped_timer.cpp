#include "scoped_timer.hpp"
#include "logger/logger.hpp"

namespace looper::time {

ScopedTimer::ScopedTimer(TimeStep* timeStep) : timeStep_(timeStep)
{
   timer_.Start();
}

ScopedTimer::ScopedTimer(std::string&& logMsg) : logMsg_(std::move(logMsg))
{
   timer_.Start();
}

// NOLINTNEXTLINE
ScopedTimer::~ScopedTimer()
{
   const auto delta = timer_.Stop();
   if (!timeStep_)
   {
      Logger::Debug("{} took {}ms", logMsg_, delta.GetMilliseconds().count());
   }
   else
   {
      (*timeStep_) = delta;
   }
   
}

} // namespace looper::time
