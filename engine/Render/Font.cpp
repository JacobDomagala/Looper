#include "Font.hpp"
#include "FileManager.hpp"
#include "Game.hpp"
#include "Texture.hpp"
#include "Window.hpp"

#include <ft2build.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include FT_FREETYPE_H

namespace dgame {

void
Font::SetFont(const std::string& fontName)
{
   m_logger.Init("Font");

   FT_Library ft;

   if (FT_Init_FreeType(&ft))
      m_logger.Log(Logger::TYPE::FATAL, "Error initializing FreeType!");

   FT_Face face;
   std::string filePath = fmt::format("{}.ttf", (FONTS_DIR / fontName).string());

   if (FT_New_Face(ft, filePath.c_str(), 0, &face))
      m_logger.Log(Logger::TYPE::FATAL, "Error loading font " + filePath);

   FT_New_Face(ft, filePath.c_str(), 0, &face);
   // Set size to load glyphs as
   FT_Set_Pixel_Sizes(face, 0, 48);

   // Disable byte-alignment restriction
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   // Load first 128 characters of ASCII set
   for (GLubyte c = 0; c < 128; ++c)
   {
      // Load character glyph
      if (FT_Load_Char(face, c, FT_LOAD_RENDER))
         m_logger.Log(Logger::TYPE::FATAL, "Error loading font face for character {} for font {}",
                      c, filePath);

      // FT_Load_Char(face, c, FT_LOAD_RENDER);
      // Generate texture
      GLuint texture;
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0,
                   GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
      // Set texture options
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      // Now store character for later use

      Character character = {texture,
                             glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                             glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                             static_cast< GLuint >(face->glyph->advance.x)};

      m_characters.insert(std::pair< GLchar, Character >(c, character));
   }

   glBindTexture(GL_TEXTURE_2D, 0);
   // Destroy FreeType once we're finished
   FT_Done_Face(face);
   FT_Done_FreeType(ft);

   glGenVertexArrays(1, &m_VAO);
   glGenBuffers(1, &m_VBO);
   glBindVertexArray(m_VAO);
   glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

   m_logger.Log(Logger::TYPE::INFO, fmt::format("Loaded font: {}", fontName));
}

void
Font::RenderText(const glm::mat4& /*projectionMatrix*/, std::string text, glm::vec2 position,
                 GLfloat scale, const glm::vec3&)
{
   // Activate corresponding render state
   /*m_program.UseProgram();
   m_program.SetUniformFloatVec4(glm::vec4(color, 1.0f), "color");
   m_program.SetUniformFloatMat4(projectionMatrix, "projectionMatrix");*/
   glActiveTexture(GL_TEXTURE0);
   glBindVertexArray(m_VAO);

   // Iterate through all characters
   std::string::const_iterator c;
   for (c = text.begin(); c != text.end(); ++c)
   {
      Character ch = m_characters[*c];

      GLfloat w = static_cast< float >(ch.size.x) * scale;
      GLfloat h = static_cast< float >(ch.size.y) * scale;

      GLfloat xpos = position.x + static_cast< float >(ch.bearing.x) * scale;
      GLfloat ypos =
         position.y + static_cast< float >(m_characters['H'].bearing.y - ch.bearing.y) * scale;

      // Seems like it stores tex coords in Direct3d style FeelsBadMan
      GLfloat vertices[6][4] = {
         {xpos, ypos + h, 0.0, 1.0}, {xpos + w, ypos, 1.0, 0.0},     {xpos, ypos, 0.0, 0.0},

         {xpos, ypos + h, 0.0, 1.0}, {xpos + w, ypos + h, 1.0, 1.0}, {xpos + w, ypos, 1.0, 0.0}};

      // Render glyph texture over quad
      glBindTexture(GL_TEXTURE_2D, ch.textureID);

      // Update content of VBO memory
      glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices),
                      vertices); // Be sure to use glBufferSubData and not glBufferData

      glBindBuffer(GL_ARRAY_BUFFER, 0);

      // Render quad
      glDrawArrays(GL_TRIANGLES, 0, 6);

      // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
      position.x += static_cast< float >(ch.advance >> 6)
                    * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of
                             // 1/64th pixels by 64 to get amount of pixels))
   }
   glBindVertexArray(0);
   glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace dgame
