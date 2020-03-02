#include "FileManager.hpp"

#include <fstream>

Logger FileManager::m_logger = Logger("FileManager");

std::string
FileManager::ReadFile(const std::string& fileName, FileType type)
{
   std::ifstream fileHandle;
   fileHandle.open(fileName, std::ifstream::in);

   if (!fileHandle.is_open())
   {
      m_logger.Log(Logger::TYPE::FATAL, (SHADERS_DIR/fileName).u8string() + " can't be opened!");
   }

   std::string returnVal((std::istreambuf_iterator< char >(fileHandle)), std::istreambuf_iterator< char >());
   fileHandle.close();

   return returnVal;
}