#pragma once

#include <chrono>
#include <string>

namespace dgame {

class Timer
{
 public:
   using milliseconds = std::chrono::milliseconds;
   using seconds = std::chrono::seconds;

   Timer();
   ~Timer() = default;

   static void
   PauseAllTimers();

   static void
   ResumeAllTimers();

   static bool
   AreTimersRunning();

   // It does 3 things:
   // - Set m_deltaTime duration (currentTime - m_timeStamp)
   // - Set m_timeStamp to current time
   // - Update m_totalTime with m_deltaTime value
   void
   ToggleTimer();

   // Get time elapsed between calling ToggleTimer() functions
   // Returned value is in miliseconds (float value)
   float
   GetFloatDeltaTime() const;

   milliseconds
   GetMsDeltaTime() const;

   // Returned value in seconds
   float
   GetTotalTime() const;

   void
   ResetTotalTime();

   static std::string
   GetCurrentTime();

   static milliseconds 
   ConvertToMs(seconds);

 private:
   // Time period between last Toggle() function call
   milliseconds m_deltaTime;

   // Total time
   milliseconds m_totalTime;

   // Used to tore temporary timepoint which is then used to calculate time between
   // each Toggle() function call
   std::chrono::steady_clock::time_point m_timeStamp;

   static inline bool m_timersPaused = false;
};

} // namespace dgame