#pragma once

#include "Timer.hpp"

#include <fmt/format.h>
#include <string>

namespace dgame {

class Logger
{
 public:
   enum class TYPE
   {
      TRACE = 0,
      DEBUG,
      INFO,
      WARNING,
      FATAL
   };

   Logger(const std::string& name);
   Logger() = default;
   ~Logger() = default;

   void
   Init(const std::string& name);

   template < typename... Args >
   void
   Log(TYPE type, const std::string& buffer, const Args&... args) const
   {
      if (type >= m_currentLogType)
      {
         fmt::vprint("{}{}<{}> ", fmt::make_format_args(Timer::GetCurrentTime(), ToString(type), m_moduleName));
         fmt::vprint(buffer, fmt::make_format_args(args...));
         fmt::print("\n");
      }
   }

   static void SetLogType(TYPE);

   std::string
   ToString(const Logger::TYPE& type) const;

 private:
   std::string m_moduleName;
   static inline TYPE m_currentLogType = Logger::TYPE::DEBUG;
};

} // namespace dgame