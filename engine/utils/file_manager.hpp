#pragma once

#include "logger.hpp"

#include <filesystem>
#include <glm/glm.hpp>
#undef max
#undef min
#include <nlohmann/json.hpp>
#include <string_view>

namespace looper {

// NOLINTBEGIN
const auto ROOT_DIR = std::filesystem::path(std::string(CMAKE_ROOT_DIR));
const auto ASSETS_DIR = ROOT_DIR / "assets" / "";
const auto LEVELS_DIR = ASSETS_DIR / "levels" / "";
const auto FONTS_DIR = ASSETS_DIR / "fonts" / "";
const auto SHADERS_DIR = ASSETS_DIR / "shaders" / "";
const auto IMAGES_DIR = ASSETS_DIR / "images" / "";
// NOLINTEND
class FileManager
{
 public:
   //NOLINTNEXTLINE
   using ImageHandleType = std::unique_ptr< uint8_t[], std::function< void(uint8_t*) > >;

   struct ImageData
   {
      ImageHandleType m_bytes = {};
      glm::ivec2 m_size = {};
      int32_t m_format = {};
   };

   enum class FileType
   {
      BINARY = 0,
      TEXT
   };

   static std::vector< char >
   ReadBinaryFile(const std::filesystem::path& path);

   static std::vector< char >
   ReadBinaryFile(std::string_view fileName);

   static std::string
   ReadFile(const std::string& fileName, FileType type = FileType::TEXT);

   static ImageData
   LoadImageData(std::string_view fileName);

   static void
   SaveImageFile(std::string_view fileName, const ImageData& image);

   static nlohmann::json
   LoadJsonFile(std::string_view pathToFile);

   static void
   SaveJsonFile(std::string_view pathToFile, const nlohmann::json& json);

   /**
    * \brief Return file path to selected file
    *
    * \param[in] defaultPath Where to open FileDialog
    * \param[in] fileTypes List of file types that we're looking for
    * Example: {{"PNG texture", ".png"}, {"PNG texture", ".jpg"}}
    *
    * \param[in] save Whether to save given file
    *
    * \return Path to selected file
    */
   static std::string
   FileDialog(const std::filesystem::path& defaultPath,
              const std::vector< std::pair< std::string, std::string > >& fileTypes, bool save);
};

} // namespace looper
