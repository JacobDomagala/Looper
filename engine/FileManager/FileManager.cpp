#include "FileManager.hpp"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <fstream>

std::string
FileManager::ReadFile(const std::string& fileName, FileType type)
{
   std::ifstream fileHandle;
   fileHandle.open(fileName, std::ifstream::in);

   if (!fileHandle.is_open())
   {
      m_logger.Log(Logger::TYPE::FATAL, "FileManager::ReadFile -> " + fileName + " can't be opened!");
   }

   std::string returnVal((std::istreambuf_iterator< char >(fileHandle)), std::istreambuf_iterator< char >());
   fileHandle.close();

   if (returnVal.empty())
   {
      m_logger.Log(Logger::TYPE::FATAL, "FileManager::ReadFile -> " + fileName + " is empty!");
   }

   return returnVal;
}

FileManager::Picture
FileManager::LoadPicture(const std::string& fileName)
{
   int force_channels = 0;
   int w, h, n;

   pictureHandleType textureData(stbi_load(fileName.c_str(), &w, &h, &n, force_channels), stbi_image_free);

   if (!textureData)
   {
      m_logger.Log(Logger::TYPE::FATAL, "FileManager::LoadPicture -> " + fileName + " can't be opened!");
   }

   return {std::move(textureData), {w,h}, n};
}

uint8_t*
FileManager::LoadPictureRawBytes(const std::string& fileName)
{
   int force_channels = 0;
   int w, h, n;

   uint8_t* textureData = stbi_load(fileName.c_str(), &w, &h, &n, force_channels);

   if (!textureData)
   {
      m_logger.Log(Logger::TYPE::FATAL, "FileManager::LoadPictureRawBytes -> " + fileName + " can't be opened!");
   }

   return textureData;
}

FileManager::PictureRaw
FileManager::LoadPictureRawData(const std::string& fileName)
{
   int force_channels = 0;
   int w, h, n;

   uint8_t* textureData = stbi_load(fileName.c_str(), &w, &h, &n, force_channels);

   if (!textureData)
   {
      m_logger.Log(Logger::TYPE::FATAL, "FileManager::LoadPictureRawData -> " + fileName + " can't be opened!");
   }

   return {std::move(textureData), {w,h}, n};
}