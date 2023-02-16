#include "logger.hpp"

namespace looper {

void
Logger::SetType(TYPE newType)
{
   s_currentLogType = newType;
}

std::string
Logger::ToString(TYPE type)
{
   std::string returnValue;

   if (type == TYPE::TRACE)
   {
      returnValue = "  [TRACE]  ";
   }
   else if (type == TYPE::DEBUG)
   {
      returnValue = "  [DEBUG]  ";
   }
   else if (type == TYPE::INFO)
   {
      returnValue = "  [INFO]   ";
   }
   else if (type == TYPE::WARNING)
   {
      returnValue = " [WARNING] ";
   }
   else if (type == TYPE::FATAL)
   {
      returnValue = "  [FATAL]  ";
   }
   else
   {
      returnValue = " [UNKNOWN] ";
   }

   return returnValue;
}

} // namespace looper::trace
