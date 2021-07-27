#pragma once

#include "Logger.hpp"
#include "Texture.hpp"

#include <glad/glad.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace dgame {

class TextureLibrary
{
 public:
   TextureLibrary() = default;
   ~TextureLibrary() = default;

   static std::shared_ptr< Texture >
   GetTexture(const std::string& textureName);

   static void
   Clear();

 private:
   static void
   LoadTexture(const std::string& textureName);

 private:
   static inline std::unordered_map< std::string, std::shared_ptr< Texture > > s_loadedTextures =
      {};
   static inline Logger s_logger = Logger("TextureLibrary");
};

} // namespace dgame