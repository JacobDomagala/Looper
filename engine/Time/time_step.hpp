#pragma once

#include <chrono>
#include <string>

namespace looper::time {
using milliseconds = std::chrono::milliseconds;
using seconds = std::chrono::seconds;

struct TimeStep
{
   explicit TimeStep(milliseconds time = {});

   std::string
   ToString() const;

   seconds
   GetSeconds() const;

   milliseconds
   GetMilliseconds() const;

 private:
   milliseconds m_time;
};

}
