#pragma once

#include "time_type.hpp"

#include <chrono>
#include <string>

namespace looper::time {

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
