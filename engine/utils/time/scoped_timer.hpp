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

   explicit ScopedTimer(std::string&& logMsg);
   ~ScopedTimer();

 private:
   std::string m_logMsg;
   Stopwatch m_timer;
};

// NOLINTNEXTLINE
#define SCOPED_TIMER(str) looper::time::ScopedTimer t(std::move(str));

} // namespace looper::time
