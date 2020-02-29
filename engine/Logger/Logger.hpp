#pragma once

#include <iostream>
#include <string>
#include <vector>

class Logger
{
 public:
   enum class TYPE
   {
      DEBUG,
      INFO,
      WARNING,
      FATAL
   };

   Logger() = default;
   ~Logger() = default;

   void
   Log(TYPE, const std::string& logBuffer) const;

   void SetLogType(TYPE);

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
   std::string m_internalBuffer;
   TYPE m_currentLogType = TYPE::DEBUG;
};
