#include "sprite.hpp"
#include "application.hpp"
#include "renderer.hpp"
#include "texture.hpp"

#include <glm/gtx/transform.hpp>

namespace looper::renderer {

void
Sprite::SetSprite(const glm::vec2& position, const glm::vec2& size)
{
   // m_texture = std::make_shared< Texture >();
   // m_texture->CreateColorTexture(size, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

   m_currentState.m_currentPosition = position;
   m_initialPosition = glm::vec3(position, 0.0f);
   m_size = size;

   m_currentState.m_translateVal = glm::vec3(position, 0.0f);
   m_currentState.m_angle = 0.0f;
   m_currentState.m_scaleVal = glm::vec2(1.0f, 1.0f);
   m_currentState.m_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
}


void
Sprite::SetSpriteTextured(const glm::vec3& position, const glm::vec2& size,
                          const std::string& fileName, ObjectType type)
{
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
   const std::vector< renderer::Vertex > vtcs = {
      {glm::vec3{-0.5f, 0.5f, m_initialPosition.z}, glm::vec3{0.0f, 0.0f, 1.0f}},
      {glm::vec3{0.5f, 0.5f, m_initialPosition.z}, glm::vec3{1.0f, 0.0f, 1.0f}},
      {glm::vec3(0.5f, -0.5f, m_initialPosition.z), glm::vec3{1.0f, 1.0f, 1.0f}},
      {glm::vec3{-0.5f, -0.5f, m_initialPosition.z}, glm::vec3{0.0f, 1.0f, 1.0f}}};

   const auto transformMat =
      glm::translate(glm::mat4(1.0f), glm::vec3(m_currentState.m_translateVal, 0.0f))
      // glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))
      * glm::rotate(glm::mat4(1.0f), m_currentState.m_angle, {0.0f, 0.0f, 1.0f})
      * glm::scale(glm::mat4(1.0f), {m_size, 1.0f});

   const std::array< std::string, 3 > txts = {
      TextureLibrary::GetTexture(texture_)->GetName(),
      TextureLibrary::GetTexture(texture_)->GetName(),
      TextureLibrary::GetTexture(texture_)->GetName()};

   rendererIdx_ =
      VulkanRenderer::MeshLoaded(vtcs, txts, transformMat, m_currentState.m_color, type);
}

void
Sprite::SetSpriteTextured(const glm::vec2& position, const glm::vec2& size,
                          const std::string& fileName, ObjectType type)
{
   SetSpriteTextured(glm::vec3{position, 0.0f}, size, fileName, type);
}

void
Sprite::Update(bool isReverse)
{
   if (isReverse)
   {
      m_currentState = m_statesQueue.back();
      m_statesQueue.pop_back();
   }
   else
   {
      m_statesQueue.push_back(m_currentState);
      if (m_statesQueue.size() >= NUM_FRAMES_TO_SAVE)
      {
         m_statesQueue.pop_front();
      }
   }
}

void
Sprite::Render()
{
   if (changed_)
   {
      const glm::mat4 transformMat =
         glm::translate(glm::mat4(1.0f),
                        glm::vec3(m_currentState.m_translateVal, m_initialPosition.z))
         * glm::rotate(glm::mat4(1.0f), m_currentState.m_angle, {0.0f, 0.0f, 1.0f})
         * glm::scale(glm::mat4(1.0f), {m_size * m_currentState.modifiers.scale, 1.0f});

      renderer::VulkanRenderer::SubmitMeshData(rendererIdx_, transformMat, m_currentState.m_color);

      changed_ = false;
   }
}

glm::vec2
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

glm::vec2
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
   m_currentState.m_color = color;
   changed_ = true;
}

void
Sprite::SetTextureFromFile(const std::string& /*filePath*/)
{
   // m_texture = TextureLibrary::GetTexture(filePath);
}

void
Sprite::SetTranslateValue(const glm::vec2& translateBy)
{
   m_currentState.m_currentPosition = glm::vec2(m_initialPosition) + translateBy;
   m_currentState.m_translateVal = m_currentState.m_currentPosition;

   changed_ = true;
}

void
Sprite::SetInitialPosition(const glm::vec2& globalPosition)
{
   m_initialPosition = glm::vec3(globalPosition, m_initialPosition.z);
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
Sprite::Translate(const glm::vec2& translateValue)
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
      glm::translate(glm::mat4(1.0f), glm::vec3(m_currentState.m_translateVal, m_initialPosition.z))
      * glm::rotate(glm::mat4(1.0f), m_currentState.m_angle, {0.0f, 0.0f, 1.0f})
      * glm::scale(glm::mat4(1.0f), {m_size, 1.0f});

   const glm::vec2 topLeft = transformMat * glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f);
   const glm::vec2 bottomLeft = transformMat * glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f);
   const glm::vec2 topRight = transformMat * glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
   const glm::vec2 bottomRight = transformMat * glm::vec4(0.5f, -0.5f, 0.0f, 1.0f);

   return {topRight, topLeft, bottomLeft, bottomRight};
}

} // namespace looper::renderer
