#include "file_manager.hpp"
#include "utils/assert.hpp"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION

#include <fstream>
#include <stb_image.h>

#if defined(_WIN32)
#include <Windows.h>
#endif //  WIN

namespace looper {

std::vector< char >
FileManager::ReadBinaryFile(const std::filesystem::path& path)
{
   return ReadBinaryFile(std::string_view{path.string()});
}

std::vector< char >
FileManager::ReadBinaryFile(std::string_view fileName)
{
   std::ifstream fileHandle(fileName.data(), std::ios::binary);

   utils::Assert(fileHandle.is_open(),
                 fmt::format("FileManager::ReadBinaryFile -> {} can't be opened!", fileName));

   const auto size = std::filesystem::file_size(fileName);

   utils::Assert(size, fmt::format("FileManager::ReadBinaryFile -> {} is empty!", fileName));

   std::vector< char > buffer(size);

   fileHandle.read(buffer.data(), static_cast< std::streamsize >(size));

   return buffer;
}

std::string
FileManager::ReadFile(const std::string& fileName, FileType /*type*/)
{
   std::ifstream fileHandle = {};
   fileHandle.open(fileName.c_str(), std::ifstream::in);

   if (!fileHandle.is_open())
   {
      Logger::Fatal("FileManager::ReadFile -> {} can't be opened!", fileName);
   }

   std::string returnVal((std::istreambuf_iterator< char >(fileHandle)),
                         std::istreambuf_iterator< char >());
   fileHandle.close();

   if (returnVal.empty())
   {
      Logger::Fatal("FileManager::ReadFile -> {} is empty!", fileName);
   }

   return returnVal;
}

FileManager::ImageData
FileManager::LoadImageData(std::string_view fileName)
{
   const auto pathToImage = std::filesystem::path(IMAGES_DIR / fileName).string();
   const int force_channels = STBI_rgb_alpha;
   int w = 0;
   int h = 0;
   int n = 0;

   ImageHandleType textureData(stbi_load(pathToImage.c_str(), &w, &h, &n, force_channels),
                               stbi_image_free);

   if (!textureData)
   {
      Logger::Fatal("FileManager::LoadImage -> {} can't be opened!", pathToImage);
   }

   return {std::move(textureData), {w, h}, n};
}

nlohmann::json
FileManager::LoadJsonFile(std::string_view pathToFile)
{
   std::ifstream jsonFile(std::string{pathToFile});

   if (!jsonFile.is_open())
   {
      Logger::Fatal("FileManager::LoadJsonFile -> {} can't be opened!",
                   pathToFile);
   }

   nlohmann::json json;
   jsonFile >> json;

   if (json.is_null())
   {
      Logger::Fatal("FileManager::LoadJsonFile -> {} is empty!", pathToFile);
   }

   return json;
}

void
FileManager::SaveJsonFile(std::string_view pathToFile, const nlohmann::json& json)
{
   std::ofstream jsonFile(std::string{pathToFile});

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

   // NOLINTNEXTLINE
   auto* output = popen(cmd.c_str(), "r");
   assert(output); // NOLINT

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

} // namespace looper
