#pragma once

#include "Logger.hpp"

#include <filesystem>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <string>

const std::filesystem::path ROOT_DIR = std::filesystem::current_path().parent_path().parent_path();
const std::filesystem::path ASSETS_DIR = ROOT_DIR / "Assets";
const std::filesystem::path LEVELS_DIR = ASSETS_DIR / "Levels";
const std::filesystem::path FONTS_DIR = ASSETS_DIR / "Fonts";
const std::filesystem::path SHADERS_DIR = ASSETS_DIR / "Shaders";
const std::filesystem::path IMAGES_DIR = ASSETS_DIR / "Images";

class FileManager
{
 public:
   using pictureHandleType = std::unique_ptr< uint8_t[], void (*)(void*) >;

   struct Picture
   {
      pictureHandleType m_bytes;
      glm::ivec2 m_size;
      int32_t m_format;
   };

   struct PictureRaw
   {
      uint8_t* m_bytes;
      glm::ivec2 m_size;
      int32_t m_format;
   };

   enum class FileType
   {
      BINARY = 0,
      TEXT
   };

   static std::string
   ReadFile(const std::string& fileName, FileType type = FileType::TEXT);

   static void
   WriteToFile(const std::string& fileName, FileType type = FileType::TEXT);

   static Picture
   LoadPicture(const std::string& fileName);

   // NOTE!
   // image data is not freed
   // TODO: Handle freeing the data
   static uint8_t*
   LoadPictureRawBytes(const std::string& fileName);

   // NOTE!
   // image data is not freed
   // TODO: Handle freeing the data
   static PictureRaw
   LoadPictureRawData(const std::string& fileName);

   static nlohmann::json
   LoadJsonFile(const std::string& fileName);

   static void
   SaveJsonFile(const std::string& fileName, nlohmann::json json);

 private:
   static inline Logger m_logger = Logger("FileManager");
};
