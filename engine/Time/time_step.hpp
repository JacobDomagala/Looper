#pragma once

#include <chrono>
#include <string>

namespace looper::time {

using milliseconds = std::chrono::milliseconds;
using seconds = std::chrono::seconds;

struct TimeStep
{
   explicit TimeStep(milliseconds time = {});

   [[nodiscard]] std::string
   ToString() const;

   [[nodiscard]] seconds
   GetSeconds() const;

   [[nodiscard]] milliseconds
   GetMilliseconds() const;

 private:
   milliseconds m_time;
};

}
