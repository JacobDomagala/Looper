#pragma once

#include <iostream>
#include <string>
#include <vector>

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

   Logger(const std::string& name)
   {
      m_moduleName = name;
   }
   Logger() = default;
   ~Logger() = default;

   void
   Init(const std::string& name);

   void
   Log(TYPE, const std::string& logBuffer) const;

   static void SetLogType(TYPE);

   friend std::ostream&
   operator<<(std::ostream& os, const Logger::TYPE& type)
   {
      if (type == Logger::TYPE::DEBUG)
      {
         os << " [DEBUG] ";
      }
      else if (type == Logger::TYPE::INFO)
      {
         os << " [INFO] ";
      }
      else if (type == Logger::TYPE::WARNING)
      {
         os << " [WARNING] ";
      }
      else if (type == Logger::TYPE::FATAL)
      {
         os << " [FATAL] ";
      }

      return os;
   }

 private:
   std::string m_moduleName;
   static TYPE m_currentLogType;
};
