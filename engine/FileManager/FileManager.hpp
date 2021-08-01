#pragma once

#include "Logger.hpp"

#include <filesystem>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <string>

namespace dgame {

const std::filesystem::path ROOT_DIR = std::filesystem::path(std::string(CMAKE_ROOT_DIR));
const std::filesystem::path ASSETS_DIR = ROOT_DIR / "assets" / "";
const std::filesystem::path LEVELS_DIR = ASSETS_DIR / "levels" / "";
const std::filesystem::path FONTS_DIR = ASSETS_DIR / "fonts" / "";
const std::filesystem::path SHADERS_DIR = ASSETS_DIR / "shaders" / "";
const std::filesystem::path IMAGES_DIR = ASSETS_DIR / "images" / "";

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

   enum class FileType
   {
      BINARY = 0,
      TEXT
   };

   static std::string
   ReadFile(const std::string& fileName, FileType type = FileType::TEXT);

   static ImageSmart
   LoadImageData(const std::string& fileName);

   static nlohmann::json
   LoadJsonFile(const std::string& pathToFile);

   static void
   SaveJsonFile(const std::string& pathToFile, nlohmann::json json);

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

 private:
   static inline Logger m_logger = Logger("FileManager");
};

} // namespace dgame
