#pragma once

#include "Logger.hpp"

#include <filesystem>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <string>

namespace dgame {

const std::filesystem::path ROOT_DIR = std::filesystem::path(std::string(CMAKE_ROOT_DIR));
const std::filesystem::path ASSETS_DIR = ROOT_DIR / "Assets";
const std::filesystem::path LEVELS_DIR = ASSETS_DIR / "Levels";
const std::filesystem::path FONTS_DIR = ASSETS_DIR / "Fonts";
const std::filesystem::path SHADERS_DIR = ASSETS_DIR / "Shaders";
const std::filesystem::path IMAGES_DIR = ASSETS_DIR / "Images";

class FileManager
{
 public:
   template < typename T > struct ImageData
   {
      T m_bytes;
      glm::ivec2 m_size;
      int32_t m_format;
   };

   using ImageHandleType = std::unique_ptr< uint8_t[], std::function< void(uint8_t*) > >;
   using ImageSmart = ImageData< ImageHandleType >;
   using ImageRaw = ImageData< uint8_t* >;

   enum class FileType
   {
      BINARY = 0,
      TEXT
   };

   static std::string
   ReadFile(const std::string& fileName, FileType type = FileType::TEXT);

   static void
   WriteToFile(const std::string& fileName, FileType type = FileType::TEXT);

   // NOTE!
   // image data is not freed
   // TODO: Handle freeing the data
   static uint8_t*
   LoadImageRawBytes(const std::string& fileName);

   static ImageSmart
   LoadImageData(const std::string& fileName);

   // NOTE!
   // image data is not freed
   // TODO: Handle freeing the data
   static ImageRaw
   LoadImageRawData(const std::string& fileName);

   static nlohmann::json
   LoadJsonFile(const std::string& fileName);

   static void
   SaveJsonFile(const std::string& fileName, nlohmann::json json);

 private:
   static inline Logger m_logger = Logger("FileManager");
};

} // namespace dgame
