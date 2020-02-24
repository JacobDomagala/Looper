#include "FileManager.hpp"

#include <fstream>

std::string
FileManager::ReadFile(const std::string& fileName, FileType type)
{
   std::ifstream fileHandle;
   fileHandle.open(fileName, std::ifstream::in);

   if (!fileHandle.is_open())
   {
      //Win_Window::GetInstance().ShowError((SHADERS_DIR/fileName).u8string() + " can't be opened!", "Opening shader file");
   }

   std::string returnVal((std::istreambuf_iterator< char >(fileHandle)), std::istreambuf_iterator< char >());
   fileHandle.close();

   return returnVal;
}