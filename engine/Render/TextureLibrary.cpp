#include "TextureLibrary.hpp"

namespace dgame {

std::shared_ptr< Texture >
TextureLibrary::GetTexture(const std::string& textureName)
{
   if (m_loadedTextures.find(textureName) == m_loadedTextures.end())
   {
      m_logger.Log(Logger::TYPE::INFO, "Texture: {} not found in library. Loading it", textureName);
      LoadTexture(textureName);
   }

   return m_loadedTextures[textureName];
}

void
TextureLibrary::LoadTexture(const std::string& textureName)
{
   m_loadedTextures[textureName] = std::make_shared< Texture >(textureName);
}

} // namespace dgame