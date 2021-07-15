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
   Log(TYPE type, const std::string& buffer, Args&&... args) const
   {
      if (type >= m_currentLogType)
      {
         fmt::print("{}{}<{}> {}\n", Timer::GetCurrentTime(), ToString(type), m_moduleName,
                    fmt::format(fmt::runtime(buffer), std::forward< Args >(args)...));
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
