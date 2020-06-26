#include "TextureLibrary.hpp"

namespace dgame {

std::shared_ptr< Texture >
TextureLibrary::GetTexture(const std::string& textureName)
{
   if (s_loadedTextures.find(textureName) == s_loadedTextures.end())
   {
      s_logger.Log(Logger::TYPE::INFO, "Texture: {} not found in library. Loading it", textureName);
      LoadTexture(textureName);
   }

   return s_loadedTextures[textureName];
}

void
TextureLibrary::Clear()
{
   s_loadedTextures.clear();
}

void
TextureLibrary::LoadTexture(const std::string& textureName)
{
   s_loadedTextures[textureName] = std::make_shared< Texture >(textureName);
}

} // namespace dgame