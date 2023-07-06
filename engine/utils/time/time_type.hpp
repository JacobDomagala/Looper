#pragma once

#include <chrono>

namespace looper::time {

using Rep = float;
using microseconds = std::chrono::duration< Rep, std::micro >;
using milliseconds = std::chrono::duration< Rep, std::milli >;
using seconds = std::chrono::duration< Rep >;

} // namespace looper::time
