#include "Utils.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

#if defined(_WIN32)
#include <Windows.h>
#endif //  WIN

namespace dgame {

std::string
CustomFloatToStr(float value, int precision)
{
   std::stringstream stream;
   stream << std::fixed << std::setprecision(precision) << value;

   return stream.str();
}

bool
IsPositionClose(const glm::vec2& targetPos, const glm::vec2& startPos, const glm::vec2& currentPos,
                float mariginValue)
{
   bool xCheck = false;
   bool yCheck = false;

   const auto targetPosWithPositiveMarigin = targetPos + mariginValue;
   const auto targetPosWithNegativeMarigin = targetPos - mariginValue;

   // calculate the direction
   const auto direction = targetPos - startPos;
   if (direction.x >= 0)
   {
      xCheck = currentPos.x >= targetPosWithNegativeMarigin.x;
   }
   else
   {
      xCheck = currentPos.x <= targetPosWithPositiveMarigin.x;
   }

   if (direction.y >= 0)
   {
      yCheck = currentPos.y >= targetPosWithNegativeMarigin.y;
   }
   else
   {
      yCheck = currentPos.y <= targetPosWithPositiveMarigin.y;
   }

   return xCheck && yCheck;
}

#if !defined(__APPLE__)
static std::vector< std::string >
file_dialog_internal(const std::vector< std::pair< std::string, std::string > >& filetypes,
                     bool save, bool multiple)
{
   static const int FILE_DIALOG_MAX_BUFFER = 16384;
   if (save && multiple)
   {
      throw std::invalid_argument("save and multiple must not both be true.");
   }

#if defined(_WIN32)
   OPENFILENAMEW ofn;
   ZeroMemory(&ofn, sizeof(OPENFILENAMEW));
   ofn.lStructSize = sizeof(OPENFILENAMEW);
   wchar_t* tmp = new wchar_t[FILE_DIALOG_MAX_BUFFER];
   ofn.lpstrFile = tmp;
   ZeroMemory(tmp, sizeof(tmp));
   ofn.nMaxFile = FILE_DIALOG_MAX_BUFFER;
   ofn.nFilterIndex = 1;

   std::string filter;

   if (!save && filetypes.size() > 1)
   {
      filter.append("Supported file types (");
      for (size_t i = 0; i < filetypes.size(); ++i)
      {
         filter.append("*.");
         filter.append(filetypes[i].first);
         if (i + 1 < filetypes.size())
            filter.append(";");
      }
      filter.append(")");
      filter.push_back('\0');
      for (size_t i = 0; i < filetypes.size(); ++i)
      {
         filter.append("*.");
         filter.append(filetypes[i].first);
         if (i + 1 < filetypes.size())
            filter.append(";");
      }
      filter.push_back('\0');
   }
   for (auto pair : filetypes)
   {
      filter.append(pair.second);
      filter.append(" (*.");
      filter.append(pair.first);
      filter.append(")");
      filter.push_back('\0');
      filter.append("*.");
      filter.append(pair.first);
      filter.push_back('\0');
   }
   filter.push_back('\0');

   int size = MultiByteToWideChar(CP_UTF8, 0, &filter[0], (int)filter.size(), NULL, 0);
   std::wstring wfilter(size, 0);
   MultiByteToWideChar(CP_UTF8, 0, &filter[0], (int)filter.size(), &wfilter[0], size);

   ofn.lpstrFilter = wfilter.data();

   if (save)
   {
      ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
      if (GetSaveFileNameW(&ofn) == FALSE)
         return {};
   }
   else
   {
      ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
      if (multiple)
         ofn.Flags |= OFN_ALLOWMULTISELECT;
      if (GetOpenFileNameW(&ofn) == FALSE)
         return {};
   }

   size_t i = 0;
   std::vector< std::string > result;
   while (tmp[i] != '\0')
   {
      std::string filename;
      int tmpSize = (int)wcslen(&tmp[i]);
      if (tmpSize > 0)
      {
         int filenameSize = WideCharToMultiByte(CP_UTF8, 0, &tmp[i], tmpSize, NULL, 0, NULL, NULL);
         filename.resize(filenameSize, 0);
         WideCharToMultiByte(CP_UTF8, 0, &tmp[i], tmpSize, &filename[0], filenameSize, NULL, NULL);
      }

      result.emplace_back(filename);
      i += tmpSize;
   }

   if (result.size() > 1)
   {
      for (i = 1; i < result.size(); ++i)
      {
         result[i] = result[0] + "\\" + result[i];
      }
      result.erase(begin(result));
   }

   delete[] tmp;
   return result;
#else
   char buffer[FILE_DIALOG_MAX_BUFFER];
   buffer[0] = '\0';

   std::string cmd = "zenity --file-selection ";
   // The safest separator for multiple selected paths is /, since / can never occur
   // in file names. Only where two paths are concatenated will there be two / following
   // each other.
   if (multiple)
      cmd += "--multiple --separator=\"/\" ";
   if (save)
      cmd += "--save ";
   cmd += "--file-filter=\"";
   for (auto pair : filetypes)
      cmd += "\"*." + pair.first + "\" ";
   cmd += "\"";
   FILE* output = popen(cmd.c_str(), "r");
   if (output == nullptr)
      throw std::runtime_error("popen() failed -- could not launch zenity!");
   while (fgets(buffer, FILE_DIALOG_MAX_BUFFER, output) != NULL)
      ;
   pclose(output);
   std::string paths(buffer);
   paths.erase(std::remove(paths.begin(), paths.end(), '\n'), paths.end());

   std::vector< std::string > result;
   while (!paths.empty())
   {
      size_t end = paths.find("//");
      if (end == std::string::npos)
      {
         result.emplace_back(paths);
         paths = "";
      }
      else
      {
         result.emplace_back(paths.substr(0, end));
         paths = paths.substr(end + 1);
      }
   }

   return result;
#endif
}
#endif

std::string
file_dialog(const std::vector< std::pair< std::string, std::string > >& filetypes, bool save)
{
   auto result = file_dialog_internal(filetypes, save, false);
   return result.empty() ? "" : result.front();
}


} // namespace dgame