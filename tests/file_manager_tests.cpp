#include "utils/file_manager.hpp"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>

int
main()
try
{
   const std::filesystem::path sourcePath = __FILE__;
   const auto expected = looper::FileManager::ReadBinaryFile(sourcePath);
   const auto path = sourcePath.string();
   const auto extendedPath = path + ".missing";
   const auto boundedPath = std::string_view{extendedPath}.substr(0, path.size());
   if (expected.empty() || looper::FileManager::ReadBinaryFile(boundedPath) != expected)
   {
      std::cerr << "Binary reads must respect the string view's length\n";
      return EXIT_FAILURE;
   }
   std::cout << "Binary file string-view path test passed\n";
}
catch (const std::exception& error)
{
   std::cerr << error.what() << '\n';
   return EXIT_FAILURE;
}
catch (...)
{
   std::cerr << "Unexpected exception\n";
   return EXIT_FAILURE;
}
