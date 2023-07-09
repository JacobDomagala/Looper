#pragma once

#include "time_step.hpp"
#include "utils/time/timer.hpp"

#include <string>

namespace looper::time {

class Stopwatch
{
 public:
   void
   Start();

   [[nodiscard]] TimeStep
   Stop();

 private:
   Timer m_timer;
};

} // namespace looper::time
