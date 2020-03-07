#pragma once

#include "Common.hpp"
#include "Shaders.hpp"
#include "Texture.hpp"

#include <GL/glew.h>
#include <deque>
#include <glm/glm.hpp>

class Window;

class Sprite
{
 public:
   Sprite() = default;
   ~Sprite() = default;

   // Create sprite without texture
   void
   SetSprite(const glm::vec2& position = glm::vec2(0.0f, 0.0f), const glm::ivec2& size = glm::ivec2(10, 10));

   // Create sprite with texture
   // Returns byte vector used for collision
   std::unique_ptr< byte_vec4 >
   SetSpriteTextured(const glm::vec2& position = glm::vec2(0.0f, 0.0f), const glm::ivec2& size = glm::ivec2(10, 10),
                     const std::string& fileName = ".\\Default.png");

   glm::vec2
   GetCenteredPosition() const;

   glm::vec2
   GetPosition() const;

   glm::ivec2
   GetSize() const;

   void
   SetColor(const glm::vec3& color);

   void
   SetTextureFromFile(const std::string& filePath);

   void
   SetTexture(const Texture& texture);

   void
   Rotate(float angle);

   void
   Scale(const glm::vec2& scaleValue);

   void
   Translate(const glm::vec2& translateValue);

   void
   Update(bool isReverse);

   // Render sprite using 'program'
   void
   Render(Window& window, const Shaders& program);

 private:
   struct State
   {
      // color of sprite (default is white)
      glm::vec4 m_color;

      // sprite's position
      glm::vec2 m_position;

      // transofmation values
      glm::vec3 m_translateVal;
      glm::vec3 m_velocity;
      glm::vec2 m_scaleVal;
      float m_angle{};
   };

   std::deque< State > m_statesQueue;
   State m_currentState;

   // sprite's texture
   Texture m_texture{};

   // color of sprite (default is white)
   glm::vec4 m_color;

   // sprite's center
   glm::vec2 m_centeredPosition;

   // OpenGL buffers
   GLuint m_vertexArrayBuffer{};
   GLuint m_vertexBuffer{};

   // width and height
   glm::ivec2 m_size;
};
