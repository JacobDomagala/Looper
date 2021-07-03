#pragma once

#include "Common.hpp"
#include "FileManager.hpp"
#include "Logger.hpp"

#include <glad/glad.h>
#include <memory>
#include <string>

namespace dgame {

class Texture
{
 public:
   Texture();
   Texture(const std::string& textureName);
   ~Texture();

   // Create new texture which is filled with 'color'
   void
   CreateColorTexture(const glm::ivec2& size, const glm::vec3& color);

   void
   LoadTextureFromFile(const std::string& fileName = "Default128.png", GLenum wrapMode = GL_REPEAT,
                       GLenum filter = GL_LINEAR);

   byte_vec4*
   GetVec4Data() const;

   uint8_t*
   GetData() const;

   std::string
   GetName() const;

   int32_t
   GetWidth() const;

   int32_t
   GetHeight() const;

   GLuint
   GetTextureHandle() const;

   // Make this texture active for given texture slot
   void
   Use(GLuint slot);

   GLuint
   Create();

   bool
   operator==(const Texture& other) const;

 private:
   // Load texture from 'data' memory
   void
   LoadTextureFromMemory(const glm::ivec2& size, uint8_t* data, const std::string& name,
                         GLenum wrapModeS = GL_CLAMP_TO_EDGE, GLenum wrapModeT = GL_CLAMP_TO_EDGE,
                         GLenum magFilter = GL_NEAREST, GLenum minFilter = GL_LINEAR);

 private:
   FileManager::ImageHandleType m_data;

   // width and size of texture
   int32_t m_width = 0;
   int32_t m_height = 0;
   int32_t m_numChannels = 0;

   std::string m_name = "EmptyName.png";

   GLuint m_textureID = 0;

   Logger m_logger = Logger("Texture");
};

} // namespace dgame