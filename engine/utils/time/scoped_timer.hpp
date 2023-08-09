#pragma once

#include "stopwatch.hpp"

#include <string>

namespace looper::time {

class ScopedTimer
{
 public:
   ScopedTimer&
   operator=(const ScopedTimer&) = delete;
   ScopedTimer&
   operator=(ScopedTimer&&) = delete;
   ScopedTimer(ScopedTimer&&) = delete;
   ScopedTimer(const ScopedTimer&) = delete;

   explicit ScopedTimer(TimeStep* timeStep);
   explicit ScopedTimer(std::string&& logMsg);
   ~ScopedTimer();

 private:
   std::string logMsg_;
   Stopwatch timer_;
   TimeStep* timeStep_ = nullptr;
};

// NOLINTNEXTLINE
#define SCOPED_TIMER(str) looper::time::ScopedTimer t(str);

} // namespace looper::time
