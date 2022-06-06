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

   explicit Logger(std::string name);
   Logger() = default;

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

   static void
   SetLogType(Type type);

   static std::string
   ToString(Type type);

 private:
   std::string m_moduleName;
   // NOLINTNEXTLINE
   static inline Type m_currentLogType = Type::DEBUG;
};

} // namespace dgame
