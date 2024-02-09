#pragma once

#include "common.hpp"
#include "state_list.hpp"
#include "texture.hpp"
#include "types.hpp"
#include "vertex.hpp"
#include "vulkan_common.hpp"

#include <glm/glm.hpp>
#include <memory>

namespace looper::renderer {

class Sprite
{
 public:
   struct Modifiers
   {
      glm::vec2 scale = {1.0f, 1.0f};
   };

   void
   ClearData() const;

   // Create sprite with texture
   void
   SetSpriteTextured(const glm::vec2& position = glm::vec2(0.0f, 0.0f),
                     const glm::vec2& size = glm::vec2(128, 128),
                     const std::string& fileName = "Default128.png", int32_t renderLayer = 2);

   void
   SetColor(const glm::vec4& color);

   void
   SetTextureFromFile(const std::string& filePath);

   void
   SetTranslateValue(const glm::vec2& translateBy);

   void
   SetInitialPosition(const glm::vec2& globalPosition);

   [[nodiscard]] glm::vec2
   GetPosition() const;

   [[nodiscard]] glm::vec2
   GetSize() const;

   void
   SetSize(const glm::vec2& newSize);

   [[nodiscard]] glm::vec2
   GetOriginalSize() const;

   [[nodiscard]] std::string
   GetTextureName() const;

   [[nodiscard]] glm::vec2
   GetTranslation() const;

   [[nodiscard]] float
   GetRotation(RotationType type = RotationType::radians) const;

   [[nodiscard]] glm::mat4
   ComputeModelMat() const;

   [[nodiscard]] glm::vec2&
   GetScale();

   [[nodiscard]] const glm::vec2&
   GetScale() const;

   [[nodiscard]] float&
   GetUniformScaleValue();

   void
   SetTextureID(TextureType type, TextureID newID);

   [[nodiscard]] const renderer::Texture*
   GetTexture() const;

   // Set rotation angle
   void
   Rotate(float angle, RotationType type = RotationType::radians);

   // Add 'angle' value to current rotation angle
   void
   RotateCumulative(float angle, RotationType type = RotationType::radians);

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

   bool
   CheckIfCollidedScreenPosion(const glm::vec3& cameraPosition,
                               const glm::vec2& globalPosition) const;

   void
   Translate(const glm::vec2& translateValue);

   void
   Update(bool isReverse);

   void
   Show();

   void
   Hide();

   void
   Render();

   void
   SetModifiers(const Modifiers& mod);

   [[nodiscard]] RenderInfo
   GetRenderInfo() const
   {
      return renderInfo_;
   }

   [[nodiscard]] int32_t
   GetRenderIdx() const
   {
      return renderInfo_.idx;
   }

   void
   SetRenderIdx(int32_t idx)
   {
      renderInfo_.idx = idx;
   }

   void
   ChangeRenderLayer(int32_t newLayer);

   static constexpr std::pair< float, float > ROTATION_RANGE = {glm::radians(-360.0f),
                                                                glm::radians(360.0f)};
   static constexpr std::pair< float, float > SCALE_RANGE = {16.0f, 1024.0f};

   glm::vec2 initialPosition_ = {};
   glm::vec2 initialSize_ = {};

 private:
   void
   ComputeBoundingBox();

   struct State
   {
      // color of sprite (default is white)
      glm::vec4 color_ = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

      // sprite's position
      glm::vec2 currentPosition_ = {0.0f, 0.0f};

      // transform values
      glm::vec2 translateVal_ = {0.0f, 0.0f};
      glm::vec2 scaleVal_ = {1.0f, 1.0f};

      // angle in radians
      float angle_ = 0.0f;

      float uniformScaleValue_ = 0.0f;

      Modifiers modifiers = {};
   };

   StateList< State > statesQueue_ = {};
   State currentState_ = {};
   SpriteType type_ = SpriteType::regular;

   // sprite's texture
   TextureIDs textures_ = {};

   // width and height
   glm::vec2 size_ = {};
   bool changed_ = false;
   RenderInfo renderInfo_ = {};

   std::vector< renderer::Vertex > vertices_ = {};
   std::array< glm::vec2, 4 > boundingBox_ = {};
};

} // namespace looper::renderer
