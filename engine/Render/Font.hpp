#pragma once

#include "FileManager.hpp"
#include "Logger.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

namespace dgame {

struct Character
{
   GLuint textureID;   // ID handle of the glyph texture
   glm::ivec2 size;    // Size of glyph
   glm::ivec2 bearing; // Offset from baseline to left/top of glyph
   GLuint advance;     // Horizontal offset to advance to next glyph
};

class Font
{
 public:
   Font() = default;
   ~Font() = default;

   void
   SetFont(const std::string& fileName);

   void
   RenderText(const glm::mat4& projectionMatrix, std::string text, glm::vec2 position,
              GLfloat scale, const glm::vec3& color);

 private:
   GLuint m_VAO{}, m_VBO{};
   std::unordered_map< GLchar, Character > m_characters;
   std::string m_shaderName{};
   Logger m_logger;
};

} // namespace dgame
