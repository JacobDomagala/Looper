#pragma once

#include "stopwatch.hpp"

#include <string>

namespace looper::time {

class ScopedTimer
{
 public:
   explicit ScopedTimer(std::string&& logMsg);
   ~ScopedTimer();

 private:
   std::string m_logMsg;
   Stopwatch m_timer;
};

#define SCOPED_TIMER(str) looper::time::ScopedTimer t(std::move(str));

} // namespace looper::time
