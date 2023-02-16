#pragma once

#include "timer.hpp"
#include "time_step.hpp"

#include <string>

namespace looper::time {

class Stopwatch
{
 public:
   void Start();

   [[nodiscard]]
   TimeStep Stop();

 private:
   Timer m_timer;
};

} // namespace shady::time
