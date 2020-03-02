#pragma once

#include "Logger.hpp"

#include <filesystem>
#include <string>

const std::filesystem::path ROOT_DIR = std::filesystem::current_path().parent_path().parent_path();
const std::filesystem::path ASSETS_DIR = ROOT_DIR / "Assets";
const std::filesystem::path SHADERS_DIR = ROOT_DIR / "Shaders";

class FileManager
{
 public:
   enum class FileType
   {
      BINARY = 0,
      TEXT
   };
   static std::string
   ReadFile(const std::string& fileName, FileType type = FileType::TEXT);

   static void
   WriteToFile(const std::string& fileName, FileType type = FileType::TEXT);

private:
   static Logger m_logger;
};
