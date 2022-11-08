#include "TextureLibrary.hpp"

namespace looper {

std::shared_ptr< Texture >
TextureLibrary::GetTexture(const std::string& textureName)
{
   if (s_loadedTextures.find(textureName) == s_loadedTextures.end())
   {
      Logger::Info("Texture: {} not found in library. Loading it", textureName);
      LoadTexture(textureName);
   }
   else {
      Logger::Info("Texture: {} found in library!", textureName);
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

} // namespace looper