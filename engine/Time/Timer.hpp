#pragma once

#include <chrono>

class Timer
{
   // Time period between last Toggle() function call
   std::chrono::milliseconds m_deltaTime;

   // Total time
   std::chrono::milliseconds m_totalTime;

   // Used to tore temporary timepoint which is then used to calculate time between
   // each Toggle() function call
   std::chrono::steady_clock::time_point m_timeStamp;

   static inline bool m_timersPaused = false;

 public:
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

   void
   Start();
   void
   Stop();

   void
   ToggleTimer()
   {
      auto currentTimeStamp = std::chrono::steady_clock::now();
      m_deltaTime = std::chrono::duration_cast< std::chrono::milliseconds >(currentTimeStamp - m_timeStamp);

      m_timeStamp = currentTimeStamp;
   }

   float
   GetDeltaTime() const
   {
      return m_deltaTime.count() / 1000.0f;
   }
   // float
   // GetGlobalTime() const
   //{
   //   return static_cast< float >(m_globalTime);
   //}
};
