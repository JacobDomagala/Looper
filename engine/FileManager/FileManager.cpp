#include "FileManager.hpp"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <fstream>
#include <stb_image.h>

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
   const auto pathToImage = std::filesystem::path(IMAGES_DIR / fileName).u8string();
   int force_channels = 0;
   int w, h, n;

   pictureHandleType textureData(stbi_load(pathToImage.c_str(), &w, &h, &n, force_channels), stbi_image_free);

   if (!textureData)
   {
      m_logger.Log(Logger::TYPE::FATAL, "FileManager::LoadPicture -> " + pathToImage + " can't be opened!");
   }

   return {std::move(textureData), {w, h}, n};
}

uint8_t*
FileManager::LoadPictureRawBytes(const std::string& fileName)
{
   const auto pathToImage = std::filesystem::path(IMAGES_DIR / fileName).u8string();
   int force_channels = 0;
   int w, h, n;

   uint8_t* textureData = stbi_load(pathToImage.c_str(), &w, &h, &n, force_channels);

   if (!textureData)
   {
      m_logger.Log(Logger::TYPE::FATAL, "FileManager::LoadPictureRawBytes -> " + pathToImage + " can't be opened!");
   }

   return textureData;
}

FileManager::PictureRaw
FileManager::LoadPictureRawData(const std::string& fileName)
{
   const auto pathToImage = std::filesystem::path(IMAGES_DIR / fileName).u8string();
   int force_channels = 0;
   int w, h, n;

   uint8_t* textureData = stbi_load(pathToImage.c_str(), &w, &h, &n, force_channels);

   if (!textureData)
   {
      m_logger.Log(Logger::TYPE::FATAL, "FileManager::LoadPictureRawData -> " + pathToImage + " can't be opened!");
   }

   return {std::move(textureData), {w, h}, n};
}

nlohmann::json
FileManager::LoadJsonFile(const std::string& pathToFile)
{
   std::ifstream jsonFile(pathToFile);

   if (!jsonFile.is_open())
   {
      m_logger.Log(Logger::TYPE::FATAL, "FileManager::LoadJsonFile -> " + pathToFile + " can't be opened!");
   }

   nlohmann::json json;
   json << jsonFile;

   if (json.is_null())
   {
      m_logger.Log(Logger::TYPE::FATAL, "FileManager::LoadJsonFile -> " + pathToFile + " is empty!");
   }

   return json;
}

void
FileManager::SaveJsonFile(const std::string& pathToFile, nlohmann::json json)
{
   std::ofstream jsonFile(pathToFile);

   jsonFile << json;
}