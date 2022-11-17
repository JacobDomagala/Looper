#pragma once

#include "logger.hpp"
#include "Texture.hpp"

#include <glad/glad.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace looper {

class TextureLibrary
{
 public:

   static std::shared_ptr< Texture >
   GetTexture(const std::string& textureName);

   static void
   Clear();

 private:
   static void
   LoadTexture(const std::string& textureName);

   // NOLINTBEGIN
   static inline std::unordered_map< std::string, std::shared_ptr< Texture > > s_loadedTextures =
      {};
   // NOLINTEND
};

} // namespace looper