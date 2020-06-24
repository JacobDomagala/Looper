#include "Logger.hpp"

#include <iostream>

namespace dgame {

Logger::Logger(const std::string& name)
{
   m_moduleName = name;
}

void
Logger::Init(const std::string& name)
{
   m_moduleName = name;
}

std::string
Logger::ToString(const Logger::TYPE& type) const
{
   std::string returnValue;

   if (type == Logger::TYPE::TRACE)
   {
      returnValue = "  [TRACE]  ";
   }
   else if (type == Logger::TYPE::DEBUG)
   {
      returnValue = "  [DEBUG]  ";
   }
   else if (type == Logger::TYPE::INFO)
   {
      returnValue = "  [INFO]   ";
   }
   else if (type == Logger::TYPE::WARNING)
   {
      returnValue = " [WARNING] ";
   }
   else if (type == Logger::TYPE::FATAL)
   {
      returnValue = "  [FATAL]  ";
   }
   else
   {
      returnValue = " [UNKNOWN] ";
   }

   return returnValue;
}

void
Logger::SetLogType(Logger::TYPE type)
{
   m_currentLogType = type;
}

} // namespace dgame