#pragma once

#include <chrono>
#include <string>

namespace looper::time {

using microseconds = std::chrono::microseconds;
using milliseconds = std::chrono::milliseconds;
using seconds = std::chrono::seconds;

struct TimeStep
{
   explicit TimeStep(milliseconds time = milliseconds{});
   explicit TimeStep(microseconds time = microseconds{});

   [[nodiscard]] std::string
   ToString() const;

   [[nodiscard]] seconds
   GetSeconds() const;

   [[nodiscard]] milliseconds
   GetMilliseconds() const;

   [[nodiscard]] microseconds
   GetMicroseconds() const;

 private:
   microseconds m_time = {};
};

} // namespace looper::time
