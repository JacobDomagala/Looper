#pragma once

#include <chrono>
#include <string>

namespace looper {

class Timer
{
 public:
   using microseconds = std::chrono::microseconds;
   using milliseconds = std::chrono::milliseconds;
   using seconds = std::chrono::seconds;

   Timer();

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
   [[nodiscard]] float
   GetFloatDeltaTime() const;

   [[nodiscard]] microseconds
   GetMicroDeltaTime() const;

   [[nodiscard]] milliseconds
   GetMsDeltaTime() const;

   // Returned value in seconds
   [[nodiscard]] float
   GetTotalTime() const;

   void
   ResetTotalTime();

   static std::string
   GetCurrentTime();

   static milliseconds ConvertToMs(seconds);

 private:
   // Time period between last Toggle() function call
   microseconds m_deltaTime;

   // Total time
   milliseconds m_totalTime;

   // Used to store temporary timepoint which is then used to calculate time between
   // each Toggle() function calls
   std::chrono::steady_clock::time_point m_timeStamp;

   // NOLINTNEXTLINE
   static inline bool m_timersPaused = false;
};

} // namespace looper