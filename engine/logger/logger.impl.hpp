#pragma once

#undef GetCurrentTime

#include "logger.hpp"
#include "time/timer.hpp"

#include <fmt/format.h>

namespace looper {


template < typename... Args >
constexpr void
Logger::Trace(fmt::format_string< Args... > buffer, Args&&... args)
{
   Log< TYPE::TRACE >(buffer, std::forward< Args >(args)...);
}

template < typename... Args >
constexpr void
Logger::Debug(fmt::format_string< Args... > buffer, Args&&... args)
{
   Log< TYPE::DEBUG >(buffer, std::forward< Args >(args)...);
}

template < typename... Args >
constexpr void
Logger::Info(fmt::format_string< Args... > buffer, Args&&... args)
{
   Log< TYPE::INFO >(buffer, std::forward< Args >(args)...);
}

template < typename... Args >
constexpr void
Logger::Warn(fmt::format_string< Args... > buffer, Args&&... args)
{
   Log< TYPE::WARNING >(buffer, std::forward< Args >(args)...);
}

template < typename... Args >
constexpr void
Logger::Fatal(fmt::format_string< Args... > buffer, Args&&... args)
{
   Log< TYPE::FATAL >(buffer, std::forward< Args >(args)...);
}

template < TYPE LogLevel, typename... Args >
constexpr void
Logger::Log(fmt::format_string<Args...> buffer, Args&&... args)
{
   if (LogLevel >= s_currentLogType)
   {
#if defined(_WIN32)
         auto hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
         SetConsoleTextAttribute(hConsole, s_typeStyles.at(LogLevel));

         fmt::print("[{}]{} {}\n", time::Timer::GetCurrentTime(), ToString(LogLevel),
                    fmt::format(buffer, std::forward< Args >(args)...));

         // Set the color to white
         SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
#else
         fmt::print(fmt::fg(s_typeStyles.at(LogLevel)), "[{}]{} {}\n", time::Timer::GetCurrentTime(),
                    ToString(LogLevel),
                    fmt::format(buffer, std::forward< Args >(args)...));
#endif
   }
}

} // namespace shady::trace
