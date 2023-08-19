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

// NOLINTBEGIN
#define TOKEN_PASTE(x, y) x##y
#define TOKEN_PASTE2(x, y) TOKEN_PASTE(x, y)
#define SCOPED_TIMER(str) looper::time::ScopedTimer TOKEN_PASTE2(timer_, __LINE__)(str);
// NOLINTEND

} // namespace looper::time
