#include "FileManager.hpp"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION

#include <fstream>
#include <stb_image.h>

#if defined(_WIN32)
#include <Windows.h>
#endif //  WIN

namespace dgame {

std::string
FileManager::ReadFile(const std::string& fileName, FileType)
{
   std::ifstream fileHandle;
   fileHandle.open(fileName, std::ifstream::in);

   if (!fileHandle.is_open())
   {
      m_logger.Log(Logger::Type::FATAL,
                   "FileManager::ReadFile -> " + fileName + " can't be opened!");
   }

   std::string returnVal((std::istreambuf_iterator< char >(fileHandle)),
                         std::istreambuf_iterator< char >());
   fileHandle.close();

   if (returnVal.empty())
   {
      m_logger.Log(Logger::Type::FATAL, "FileManager::ReadFile -> " + fileName + " is empty!");
   }

   return returnVal;
}

FileManager::ImageSmart
FileManager::LoadImageData(const std::string& fileName)
{
   const auto pathToImage = std::filesystem::path(IMAGES_DIR / fileName).string();
   int force_channels = 0;
   int w, h, n;

   ImageHandleType textureData(stbi_load(pathToImage.c_str(), &w, &h, &n, force_channels),
                               stbi_image_free);

   if (!textureData)
   {
      m_logger.Log(Logger::Type::FATAL,
                   fmt::format("FileManager::LoadImage -> {} can't be opened!", pathToImage));
   }

   return {std::move(textureData), {w, h}, n};
}

nlohmann::json
FileManager::LoadJsonFile(const std::string& pathToFile)
{
   std::ifstream jsonFile(pathToFile);

   if (!jsonFile.is_open())
   {
      m_logger.Log(Logger::Type::FATAL,
                   "FileManager::LoadJsonFile -> " + pathToFile + " can't be opened!");
   }

   nlohmann::json json;
   jsonFile >> json;

   if (json.is_null())
   {
      m_logger.Log(Logger::Type::FATAL,
                   "FileManager::LoadJsonFile -> " + pathToFile + " is empty!");
   }

   return json;
}

void
FileManager::SaveJsonFile(const std::string& pathToFile, nlohmann::json json)
{
   std::ofstream jsonFile(pathToFile);

   jsonFile << json;
}

std::string
FileManager::FileDialog(const std::filesystem::path& defaultPath,
                        const std::vector< std::pair< std::string, std::string > >& fileTypes,
                        bool save)
{
   constexpr auto FILE_DIALOG_MAX_BUFFER = 16384;

#if defined(_WIN32)
   //////////////////////////////////////////////
   ///////     Generate file filter       ///////
   //////////////////////////////////////////////
   std::string filter;

   if (!save && fileTypes.size() > 1)
   {
      auto generateFormats = [](const auto& filterTypes) {
         std::string buffer;
         for (size_t i = 0; i < filterTypes.size(); ++i)
         {
            buffer.append(
               fmt::format("*.{}{}", filterTypes[i].second, i < filterTypes.size() - 1 ? ";" : ""));
         }

         return buffer;
      };

      // There could be a simpler way to insert \0 mid string
      // Doesn't seem to work with fmt::format sadly
      const auto formats = generateFormats(fileTypes);
      filter.append(fmt::format("Supported file types ({})", formats));
      filter.push_back('\0');
      filter.append(fmt::format("{}", formats));
      filter.push_back('\0');
   }

   for (const auto& [description, extension] : fileTypes)
   {
      // Same as above, need to explicitly push_back \0
      filter.append(fmt::format("{} (*.{})", description, extension));
      filter.push_back('\0');
      filter.append(fmt::format("*.{}", extension));
      filter.push_back('\0');
   }


   filter.push_back('\0');

   const auto size =
      MultiByteToWideChar(CP_UTF8, 0, filter.c_str(), static_cast< int >(filter.size()), NULL, 0);
   std::wstring wfilter(size, 0);
   MultiByteToWideChar(CP_UTF8, 0, filter.c_str(), static_cast< int >(filter.size()),
                       wfilter.data(), size);


   //////////////////////////////////////////////
   ///////      Open file dialog          ///////
   //////////////////////////////////////////////

   std::wstring fileDialogBuffer(FILE_DIALOG_MAX_BUFFER, 0);
   const auto directoryWstring = defaultPath.wstring();

   OPENFILENAMEW ofn = {0};
   ofn.lStructSize = sizeof(OPENFILENAMEW);
   ofn.lpstrInitialDir = directoryWstring.c_str();
   ofn.lpstrFile = fileDialogBuffer.data();
   ofn.nMaxFile = FILE_DIALOG_MAX_BUFFER;
   ofn.nFilterIndex = 1;
   ofn.lpstrFilter = wfilter.c_str();

   if (save)
   {
      ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

      if (!GetSaveFileNameW(&ofn))
      {
         return {};
      }
   }
   else
   {
      ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

      if (!GetOpenFileNameW(&ofn))
      {
         return {};
      }
   }

   std::string result;

   const auto tmpSize = static_cast< int >(wcslen(fileDialogBuffer.c_str()));
   assert(tmpSize > 0);

   auto filenameSize =
      WideCharToMultiByte(CP_UTF8, 0, fileDialogBuffer.c_str(), tmpSize, NULL, 0, NULL, NULL);
   result.resize(filenameSize, 0);

   WideCharToMultiByte(CP_UTF8, 0, fileDialogBuffer.c_str(), tmpSize, result.data(), filenameSize,
                       NULL, NULL);

   return result;
#else


   const auto cmd = fmt::format("zenity --file-selection --filename={} {} --file-filter=\"{}\"",
                                defaultPath.string(), save ? "--save " : "", [&fileTypes] {
                                   std::string types;
                                   for ([[maybe_unused]] const auto& [_, extension] : fileTypes)
                                   {
                                      types.append(fmt::format("\"*.{}\" ", extension));
                                   }
                                   return types;
                                }());

   auto* output = popen(cmd.c_str(), "r");
   assert(output);

   std::string buffer(FILE_DIALOG_MAX_BUFFER, 0);
   if (fgets(buffer.data(), FILE_DIALOG_MAX_BUFFER, output))
   {
      // fgets includes \n character at the end, remove it
      buffer[strcspn(buffer.c_str(), "\n")] = 0;
   }
   else
   {
      buffer = "";
   }

   pclose(output);

   return buffer;
#endif
}

} // namespace dgame
