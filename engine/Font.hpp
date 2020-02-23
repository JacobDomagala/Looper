#pragma once

#include "Shaders.hpp"
#include "Texture.hpp"
#include "FileManager.hpp"

#include <GL/glew.h>
#include <string>
#include <unordered_map>
#include<glm/glm.hpp>

struct Character
{
   GLuint textureID;   // ID handle of the glyph texture
   glm::ivec2 size;    // Size of glyph
   glm::ivec2 bearing; // Offset from baseline to left/top of glyph
   GLuint advance;     // Horizontal offset to advance to next glyph
};

class Font
{
   GLuint VAO{}, VBO{};
   std::unordered_map< GLchar, Character > Characters;
   Shaders program{};
   /*Texture texture{};*/

 public:
   Font() = default;
   ~Font() = default;

   void
   SetFont(const std::string& fileName = (ASSETS_DIR/"segoeui.ttf").u8string());
   void
   RenderText(std::string text, glm::vec2 position, GLfloat scale, const glm::vec3& color);
};
