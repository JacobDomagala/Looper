#pragma once

#include "Timer.hpp"

#include <fmt/format.h>
#include <string>

namespace dgame {

class Logger
{
 public:
   enum class Type
   {
      TRACE = 0,
      DEBUG,
      INFO,
      WARNING,
      FATAL
   };

   explicit Logger(const std::string& name);
   Logger() = default;
   ~Logger() = default;

   void
   Init(const std::string& name);

   template < typename... Args >
   void
   Log(Type type, const std::string& buffer, Args&&... args) const
   {
      if (type >= m_currentLogType)
      {
         fmt::print("{}{}<{}> {}\n", Timer::GetCurrentTime(), ToString(type), m_moduleName,
                    fmt::format(fmt::runtime(buffer), std::forward< Args >(args)...));
      }
   }

   static void SetLogType(Type);

   std::string
   ToString(const Logger::Type& type) const;

 private:
   std::string m_moduleName;
   static inline Type m_currentLogType = Logger::Type::DEBUG;
};

} // namespace dgame
