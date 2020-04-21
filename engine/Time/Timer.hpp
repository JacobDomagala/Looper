#pragma once

#include <chrono>

class Timer
{
 public:
   using milliseconds = std::chrono::milliseconds;
   using seconds = std::chrono::seconds;

   Timer();
   ~Timer() = default;

   static void
   PauseAllTimers()
   {
      m_timersPaused = true;
   }

   static void
   ResumeAllTimers()
   {
      m_timersPaused = false;
   }

   static bool
   AreTimersRunning()
   {
      return !m_timersPaused;
   }

   // It does 3 things:
   // - Set m_deltaTime duration (currentTime - m_timeStamp)
   // - Set m_timeStamp to current time
   // - Update m_totalTime with m_deltaTime value
   void
   ToggleTimer();

   // Get time elapsed between calling ToggleTimer() functions
   // Returned value is in seconds (float value)
   float
   GetDeltaTime() const
   {
      return m_deltaTime.count() / 1000.0f;
   }

   // Returned value in seconds
   float
   GetTotalTime() const
   {
      return std::chrono::duration_cast< milliseconds >(m_totalTime).count() / 1.0f;
   }

   void
   ResetTotalTime()
   {
      m_totalTime = milliseconds(0);
   }

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
