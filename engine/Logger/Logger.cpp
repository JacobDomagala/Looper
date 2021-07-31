#include "Logger.hpp"

#include <iostream>

namespace dgame {

Logger::Logger(const std::string& name) : m_moduleName(name)
{
}

void
Logger::Init(const std::string& name)
{
   m_moduleName = name;
}

std::string
Logger::ToString(const Logger::Type& type) const
{
   std::string returnValue;

   if (type == Logger::Type::TRACE)
   {
      returnValue = "  [TRACE]  ";
   }
   else if (type == Logger::Type::DEBUG)
   {
      returnValue = "  [DEBUG]  ";
   }
   else if (type == Logger::Type::INFO)
   {
      returnValue = "  [INFO]   ";
   }
   else if (type == Logger::Type::WARNING)
   {
      returnValue = " [WARNING] ";
   }
   else if (type == Logger::Type::FATAL)
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
Logger::SetLogType(Logger::Type type)
{
   m_currentLogType = type;
}

} // namespace dgame