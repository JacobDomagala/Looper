#pragma once

#include "common.hpp"
#include "texture.hpp"
#include "types.hpp"
#include "vertex.hpp"
#include "vulkan_common.hpp"

#include <deque>
#include <glm/glm.hpp>
#include <memory>

namespace looper::renderer {

class Sprite
{
 public:
   enum class RotationType
   {
      DEGREES,
      RADIANS
   };

   struct Modifiers
   {
      glm::vec2 scale = {1.0f, 1.0f};
   };

   ~Sprite();

   // Create sprite without texture
   void
   SetSprite(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f),
             const glm::vec2& size = glm::vec2(10, 10));

   // Create sprite with texture
   void
   SetSpriteTextured(const glm::vec2& position = glm::vec2(0.0f, 0.0f),
                     const glm::vec2& size = glm::vec2(10, 10),
                     const std::string& fileName = "Default128.png");

   void
   SetSpriteTextured(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f),
                     const glm::vec2& size = glm::vec2(10, 10),
                     const std::string& fileName = "Default128.png");

   void
   SetColor(const glm::vec4& color);

   void
   SetTextureFromFile(const std::string& filePath);

   void
   SetTranslateValue(const glm::vec3& translateBy);

   void
   SetInitialPosition(const glm::vec3& globalPosition);

   [[nodiscard]] glm::vec3
   GetPosition() const;

   [[nodiscard]] glm::vec2
   GetSize() const;

   void SetSize(glm::vec2);

   [[nodiscard]] glm::vec2
   GetOriginalSize() const;

   [[nodiscard]] std::string
   GetTextureName() const;

   [[nodiscard]] glm::vec3
   GetTranslation() const;

   [[nodiscard]] float
   GetRotation(RotationType type = RotationType::RADIANS) const;

   [[nodiscard]] glm::vec2&
   GetScale();

   [[nodiscard]] float&
   GetUniformScaleValue();

   [[nodiscard]] const renderer::Texture*
   GetTexture() const;

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
   ScaleUniformly(float scaleValue);

   //        0,       1,          2,           3
   // topRight, topLeft, bottomLeft, bottomRight
   [[nodiscard]] std::array< glm::vec2, 4 >
   GetTransformedRectangle() const;

   void
   Translate(const glm::vec3& translateValue);

   void
   Update(bool isReverse);

   void
   Render();

   void
   SetModifiers(const Modifiers& mod);

   [[nodiscard]] RenderInfo
   GetRenderInfo() const
   {
      return renderInfo_;
   }

   [[nodiscard]] uint32_t
   GetRenderIdx() const
   {
      return renderInfo_.idx;
   }

   void
   SetRenderIdx(uint32_t idx)
   {
      renderInfo_.idx = idx;
   }

   static constexpr std::pair< float, float > ROTATION_RANGE = {glm::radians(-360.0f),
                                                                glm::radians(360.0f)};
   static constexpr std::pair< float, float > SCALE_RANGE = {1.0f, 5.0f};

 private:
   struct State
   {
      // color of sprite (default is white)
      glm::vec4 m_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

      // sprite's position
      glm::vec3 m_currentPosition = {0.0f, 0.0f, 0.0f};

      // transofmation values
      glm::vec3 m_translateVal = {0.0f, 0.0f, 0.0f};
      glm::vec2 m_scaleVal = {1.0f, 1.0f};

      // angle in radians
      float m_angle = 0.0f;

      float m_uniformScaleValue = 0.0f;

      Modifiers modifiers = {};
   };

   std::deque< State > m_statesQueue = {};
   State m_currentState = {};

   // sprite's texture
   renderer::TextureID texture_ = {};

   glm::vec3 m_initialPosition = {};

   // width and height
   glm::vec2 m_size = {};
   bool changed_ = false;
   RenderInfo renderInfo_ = {};

   std::vector< renderer::Vertex > vertices_ = {};
};

} // namespace looper::renderer
