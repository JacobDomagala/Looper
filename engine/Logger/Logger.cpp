#include "Logger.hpp"



void
Logger::Log(Logger::TYPE type, const std::string& logBuffer)
{
    std::cout << type << logBuffer;
}

void
Logger::SetLogType(Logger::TYPE type)
{
    m_currentLogType = type;
}