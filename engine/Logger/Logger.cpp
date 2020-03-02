#include "Logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>

Logger::TYPE Logger::m_currentLogType = Logger::TYPE::DEBUG;

void
Logger::Init(const std::string& name)
{
   m_moduleName = name;
}

void
Logger::Log(Logger::TYPE type, const std::string& logBuffer) const
{
   if (type >= m_currentLogType)
   {
      auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      std::cout << std::put_time(std::localtime(&time), "%T") << " <" << m_moduleName << '>' << type << ' ' << logBuffer << "\n";
   }
}

void
Logger::SetLogType(Logger::TYPE type)
{
   m_currentLogType = type;
}