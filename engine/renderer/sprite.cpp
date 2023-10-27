#include "sprite.hpp"
#include "application.hpp"
#include "renderer.hpp"
#include "texture.hpp"

#include <bits/ranges_algo.h>
#include <glm/gtx/transform.hpp>

namespace looper::renderer {

Sprite::~Sprite()
{
   const glm::mat4 transformMat =
      glm::translate(glm::mat4(1.0f), m_currentState.m_translateVal)
      * glm::rotate(glm::mat4(1.0f), m_currentState.m_angle, {0.0f, 0.0f, 1.0f})
      * glm::scale(glm::mat4(1.0f), {m_size * m_currentState.modifiers.scale, 1.0f});

   renderer::VulkanRenderer::SubmitMeshData(renderInfo_.idx, texture_, transformMat,
                                            {0.0f, 0.0f, 0.0f, 0.0f});

   renderer::VulkanRenderer::MeshDeleted(renderInfo_);
}

void
Sprite::ChangeRenderLayer(uint32_t newLayer)
{
   renderer::VulkanRenderer::MeshDeleted(renderInfo_);

   for (auto& vertex : vertices_)
   {
      vertex.m_position.z = static_cast< float >(newLayer) / 20.0f;
   }

   const std::array< std::string, 3 > txts = {TextureLibrary::GetTexture(texture_)->GetName(),
                                              TextureLibrary::GetTexture(texture_)->GetName(),
                                              TextureLibrary::GetTexture(texture_)->GetName()};

   const auto transformMat =
      glm::translate(glm::mat4(1.0f), m_currentState.m_translateVal)
      * glm::rotate(glm::mat4(1.0f), m_currentState.m_angle, {0.0f, 0.0f, 1.0f})
      * glm::scale(glm::mat4(1.0f), {m_size, 1.0f});

   renderInfo_ = VulkanRenderer::MeshLoaded(vertices_, txts, transformMat, m_currentState.m_color);
}

void
Sprite::SetSprite(const glm::vec3& position, const glm::vec2& size)
{
   // m_texture = std::make_shared< Texture >();
   // m_texture->CreateColorTexture(size, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

   m_currentState.m_currentPosition = position;
   m_initialPosition = position;
   m_currentState.m_translateVal = position;
   m_size = size;
   m_currentState.m_angle = 0.0f;
   m_currentState.m_scaleVal = glm::vec2(1.0f, 1.0f);
   m_currentState.m_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

void
Sprite::SetSpriteTextured(const glm::vec3& position, const glm::vec2& size,
                          const std::string& fileName)
{
   changed_ = true;
   texture_ = renderer::TextureLibrary::GetTexture(fileName)->GetID();

   m_initialPosition = position;
   m_currentState.m_currentPosition = position;
   m_size = size;

   m_currentState.m_translateVal = position;
   m_currentState.m_angle = 0.0f;
   m_currentState.m_scaleVal = glm::vec2(1.0f, 1.0f);
   m_currentState.m_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);


   /*                          y -1
                                ^
                                |
       x (0) [-0.5f, 0.5f]      |     x (1) [0.5f, 0.5f]
                                |
     -------------------------------------------------------->  x +1
                                |
                                |
        x (3) [-0.5f, -0.5f]    |     x (2) [0.5f, -0.5f]

   */
   vertices_ = std::vector< renderer::Vertex >{
      {glm::vec3{-0.5f, 0.5f, position.z}, glm::vec3{0.0f, 0.0f, 1.0f}},
      {glm::vec3{0.5f, 0.5f, position.z}, glm::vec3{1.0f, 0.0f, 1.0f}},
      {glm::vec3(0.5f, -0.5f, position.z), glm::vec3{1.0f, 1.0f, 1.0f}},
      {glm::vec3{-0.5f, -0.5f, position.z}, glm::vec3{0.0f, 1.0f, 1.0f}}};

   const auto transformMat =
      glm::translate(glm::mat4(1.0f), m_currentState.m_translateVal)
      * glm::rotate(glm::mat4(1.0f), m_currentState.m_angle, {0.0f, 0.0f, 1.0f})
      * glm::scale(glm::mat4(1.0f), {m_size, 1.0f});

   const std::array< std::string, 3 > txts = {TextureLibrary::GetTexture(texture_)->GetName(),
                                              TextureLibrary::GetTexture(texture_)->GetName(),
                                              TextureLibrary::GetTexture(texture_)->GetName()};

   renderInfo_ = VulkanRenderer::MeshLoaded(vertices_, txts, transformMat, m_currentState.m_color);
}

void
Sprite::SetSpriteTextured(const glm::vec2& position, const glm::vec2& size,
                          const std::string& fileName)
{
   SetSpriteTextured(glm::vec3{position, 0.0f}, size, fileName);
}

void
Sprite::Update(bool isReverse)
{
   if (isReverse)
   {
      m_currentState = m_statesQueue.GetLastState();

      changed_ = true;
   }
   else
   {
      m_statesQueue.PushState(m_currentState);
   }
}

void
Sprite::Render()
{
   if (changed_)
   {
      const glm::mat4 transformMat =
         glm::translate(glm::mat4(1.0f), m_currentState.m_translateVal)
         * glm::rotate(glm::mat4(1.0f), m_currentState.m_angle, {0.0f, 0.0f, 1.0f})
         * glm::scale(glm::mat4(1.0f), {m_size * m_currentState.modifiers.scale, 1.0f});

      renderer::VulkanRenderer::SubmitMeshData(renderInfo_.idx, texture_, transformMat,
                                               m_currentState.m_color);

      changed_ = false;
   }
}

glm::vec3
Sprite::GetPosition() const
{
   return m_currentState.m_currentPosition;
}

glm::vec2
Sprite::GetSize() const
{
   return m_size;
}

void
Sprite::SetSize(glm::vec2 size)
{
   m_size = size;
   changed_ = true;
}

glm::vec2
Sprite::GetOriginalSize() const
{
   return m_size;
}

std::string
Sprite::GetTextureName() const
{
   return renderer::TextureLibrary::GetTexture(texture_)->GetName();
}

glm::vec3
Sprite::GetTranslation() const
{
   return m_currentState.m_translateVal;
}

float
Sprite::GetRotation(RotationType type) const
{
   return type == RotationType::DEGREES ? glm::degrees(m_currentState.m_angle)
                                        : m_currentState.m_angle;
}

glm::vec2&
Sprite::GetScale()
{
   return m_currentState.m_scaleVal;
}

float&
Sprite::GetUniformScaleValue()
{
   return m_currentState.m_uniformScaleValue;
}

void
Sprite::SetColor(const glm::vec4& color)
{
   if (m_currentState.m_color != color)
   {
      m_currentState.m_color = color;
      changed_ = true;
   }
}

void
Sprite::SetTextureFromFile(const std::string& filePath)
{
   texture_ = renderer::TextureLibrary::GetTexture(filePath)->GetID();
   changed_ = true;
}

void
Sprite::SetTranslateValue(const glm::vec3& translateBy)
{
   m_currentState.m_currentPosition = m_initialPosition + translateBy;
   m_currentState.m_translateVal += translateBy;

   changed_ = true;
}

void
Sprite::SetInitialPosition(const glm::vec3& globalPosition)
{
   m_initialPosition = globalPosition;
}

const renderer::Texture*
Sprite::GetTexture() const
{
   return renderer::TextureLibrary::GetTexture(texture_);
}

void
Sprite::Rotate(float angle, RotationType type)
{
   m_currentState.m_angle = type == RotationType::DEGREES ? glm::degrees(angle) : angle;
   m_currentState.m_angle =
      glm::clamp(m_currentState.m_angle, glm::radians(-360.0f), glm::radians(360.0f));

   changed_ = true;
}

void
Sprite::RotateCumulative(float angle, RotationType type)
{
   m_currentState.m_angle += type == RotationType::DEGREES ? glm::degrees(angle) : angle;
   m_currentState.m_angle =
      glm::clamp(m_currentState.m_angle, glm::radians(-360.0f), glm::radians(360.0f));

   changed_ = true;
}

void
Sprite::Scale(const glm::vec2& scaleValue)
{
   m_currentState.m_scaleVal = scaleValue;
   m_currentState.m_scaleVal.x =
      glm::clamp(m_currentState.m_scaleVal.x, SCALE_RANGE.first, SCALE_RANGE.second);
   m_currentState.m_scaleVal.y =
      glm::clamp(m_currentState.m_scaleVal.y, SCALE_RANGE.first, SCALE_RANGE.second);

   m_size = static_cast< glm::vec2 >(m_size)
            * (m_currentState.m_scaleVal + m_currentState.m_uniformScaleValue);

   changed_ = true;
}

void
Sprite::ScaleCumulative(const glm::vec2& scaleValue)
{
   m_currentState.m_scaleVal += scaleValue;
   m_currentState.m_scaleVal.x =
      glm::clamp(m_currentState.m_scaleVal.x, SCALE_RANGE.first, SCALE_RANGE.second);
   m_currentState.m_scaleVal.y =
      glm::clamp(m_currentState.m_scaleVal.y, SCALE_RANGE.first, SCALE_RANGE.second);

   m_size = static_cast< glm::vec2 >(m_size)
            * (m_currentState.m_scaleVal + m_currentState.m_uniformScaleValue);

   changed_ = true;
}

void
Sprite::Translate(const glm::vec3& translateValue)
{
   m_currentState.m_currentPosition += translateValue;
   m_currentState.m_translateVal += translateValue;

   changed_ = true;
}

void
Sprite::SetModifiers(const Modifiers& mod)
{
   m_currentState.modifiers = mod;
}

void
Sprite::ScaleUniformly(const float scaleValue)
{
   m_currentState.m_uniformScaleValue = scaleValue;
   /*  m_currentState.m_scaleVal += glm::vec2(scaleValue, scaleValue);
     m_currentState.m_scaleVal.x = glm::clamp(m_currentState.m_scaleVal.x, m_scaleRange.first,
     m_scaleRange.second); m_currentState.m_scaleVal.y = glm::clamp(m_currentState.m_scaleVal.y,
     m_scaleRange.first, m_scaleRange.second);*/

   changed_ = true;
}

std::array< glm::vec2, 4 >
Sprite::GetTransformedRectangle() const
{
   const auto transformMat =
      glm::translate(glm::mat4(1.0f), m_currentState.m_translateVal)
      * glm::rotate(glm::mat4(1.0f), m_currentState.m_angle, {0.0f, 0.0f, 1.0f})
      * glm::scale(glm::mat4(1.0f), {m_size, 1.0f});

   const glm::vec2 topLeft = transformMat * glm::vec4(vertices_[0].m_position, 1.0f);
   const glm::vec2 bottomLeft = transformMat * glm::vec4(vertices_[3].m_position, 1.0f);
   const glm::vec2 topRight = transformMat * glm::vec4(vertices_[1].m_position, 1.0f);
   const glm::vec2 bottomRight = transformMat * glm::vec4(vertices_[2].m_position, 1.0f);

   return {topRight, topLeft, bottomLeft, bottomRight};
}

} // namespace looper::renderer
