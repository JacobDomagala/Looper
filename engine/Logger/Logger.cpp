#include "Logger.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>

void
Logger::Log(Logger::TYPE type, const std::string& logBuffer) const
{
    auto time =  std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout << std::put_time(std::localtime(&time), "%T") << type << logBuffer << "\n";
}

void
Logger::SetLogType(Logger::TYPE type)
{
    m_currentLogType = type;
}