#pragma once

#include "Common.hpp"
#include "Shader.hpp"
#include "TextureLibrary.hpp"

#include <deque>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace dgame {

class Window;
class Application;

class Sprite
{
 public:
   enum class RotationType
   {
      DEGREES,
      RADIANS
   };

 public:
   Sprite() = default;
   ~Sprite() = default;
   Sprite(Sprite&&) = default;

   // Create sprite without texture
   void
   SetSprite(const glm::vec2& position = glm::vec2(0.0f, 0.0f),
             const glm::ivec2& size = glm::ivec2(10, 10));

   // Create sprite with texture
   // Returns byte data used for collision
   byte_vec4*
   SetSpriteTextured(const glm::vec2& position = glm::vec2(0.0f, 0.0f),
                     const glm::ivec2& size = glm::ivec2(10, 10),
                     const std::string& fileName = "Default128.png");

   void
   SetColor(const glm::vec3& color);

   void
   SetTextureFromFile(const std::string& filePath);

   void
   SetTexture(const Texture& texture);

   void
   SetTranslateValue(const glm::vec2& translateBy);

   void
   SetInitialPosition(const glm::vec2& globalPosition);

   glm::vec2
   GetPosition() const;

   glm::ivec2
   GetSize() const;

   void SetSize(glm::vec2);

   glm::vec2
   GetOriginalSize() const;

   std::string
   GetTextureName() const;

   glm::vec2
   GetTranslation() const;

   float
   GetRotation(RotationType type = RotationType::RADIANS) const;

   glm::vec2&
   GetScale();

   float&
   GetUniformScaleValue();

   Texture&
   GetTexture();

   // Set rotation angle
   void
   Rotate(float angle, RotationType type = RotationType::RADIANS);

   // Add 'angle' value to current rotation angle
   void
   RotateCumulative(float angle, RotationType type = RotationType::RADIANS);

   void
   Scale(const glm::vec2& scaleValue);

   void
   ScaleCumulative(const glm::vec2& scaleValue);

   void
   ScaleUniformly(const float scaleValue);

   //        0,       1,          2,           3
   // topRight, topLeft, bottomLeft, bottomRight
   std::array< glm::vec2, 4 >
   GetTransformedRectangle() const;

   void
   Translate(const glm::vec2& translateValue);

   void
   Update(bool isReverse);

   void
   Render();

 public:
   static inline const std::pair< float, float > s_ROTATIONRANGE = {glm::radians(-360.0f),
                                                                    glm::radians(360.0f)};
   static inline const std::pair< float, float > s_SCALERANGE = {1.0f, 5.0f};

 private:
   struct State
   {
      // color of sprite (default is white)
      glm::vec4 m_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

      // sprite's position
      glm::vec2 m_currentPosition = {0.0f, 0.0f};

      // transofmation values
      glm::vec2 m_translateVal = {0.0f, 0.0f};
      glm::vec2 m_scaleVal = {1.0f, 1.0f};

      // angle in radians
      float m_angle = 0.0f;

      float m_uniformScaleValue = 0.0f;
   };

   std::deque< State > m_statesQueue;
   State m_currentState;

   // sprite's texture
   std::shared_ptr< Texture > m_texture;

   glm::vec2 m_initialPosition;

   // width and height
   glm::ivec2 m_size;
};

} // namespace dgame
