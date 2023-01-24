#include "stopwatch.hpp"
#include "Logger/logger.hpp"

namespace looper::time {

void Stopwatch::Start(){
   m_timer.ToggleTimer();
}

TimeStep Stopwatch::Stop(){
   m_timer.ToggleTimer();
   return TimeStep{m_timer.GetMsDeltaTime()};
}

} // namespace shady::time
