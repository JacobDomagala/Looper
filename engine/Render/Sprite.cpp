#include "Sprite.hpp"
#include "Application.hpp"
#include "Renderer.hpp"

#include <glm/gtx/transform.hpp>

namespace dgame {

void
Sprite::SetSprite(const glm::vec2& position, const glm::ivec2& size)
{
   m_texture = std::make_shared< Texture >();
   m_texture->CreateColorTexture(size, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

   m_currentState.m_currentPosition = position;
   m_initialPosition = position;
   m_size = size;

   m_currentState.m_translateVal = glm::vec3(position, 0.0f);
   m_currentState.m_angle = 0.0f;
   m_currentState.m_scaleVal = glm::vec2(1.0f, 1.0f);
   m_currentState.m_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

byte_vec4*
Sprite::SetSpriteTextured(const glm::vec2& position, const glm::ivec2& size,
                          const std::string& fileName)
{
   m_texture = TextureLibrary::GetTexture(fileName);
   auto returnPtr = reinterpret_cast< byte_vec4* >(
      m_texture->GetData()); // m_texture->LoadTextureFromFile(fileName);

   /*glGenVertexArrays(1, &m_vertexArrayBuffer);
   glGenBuffers(1, &m_vertexBuffer);
   glBindVertexArray(m_vertexArrayBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

   glm::vec4 positions[] = {glm::vec4(0.0f + size.x, 0.0f, 1.0f, 1.0f),         glm::vec4(0.0f,
   0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f - size.y, 0.0f, 0.0f),

                            glm::vec4(0.0f + size.x, 0.0f, 1.0f, 1.0f),         glm::vec4(0.0f, 0.0f
   - size.y, 0.0f, 0.0f), glm::vec4(0.0f + size.x, 0.0f - size.y, 1.0f, 0.0f)};*/

   /*m_centeredPosition.x = position.x + (size.x / 2.0f);
   m_centeredPosition.y = position.y - (size.y / 2.0f);*/
   m_initialPosition = position;
   m_currentState.m_currentPosition = position;
   m_size = size;

   m_currentState.m_translateVal = glm::vec3(position, 0.0f);
   m_currentState.m_angle = 0.0f;
   m_currentState.m_scaleVal = glm::vec2(1.0f, 1.0f);
   m_currentState.m_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

   /*glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
   glBindVertexArray(0);*/

   return returnPtr;
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
   Renderer::DrawQuad(m_currentState.m_translateVal, m_size, m_currentState.m_angle,
                      TextureLibrary::GetTexture(m_texture->GetName()), 1.0f,
                      m_currentState.m_color);
}

glm::vec2
Sprite::GetPosition() const
{
   return m_currentState.m_currentPosition;
}

glm::ivec2
Sprite::GetSize() const
{
   return m_size;
}

void
Sprite::SetSize(glm::vec2 size)
{
   m_size = size;
}

glm::vec2
Sprite::GetOriginalSize() const
{
   return m_size;
}

std::string
Sprite::GetTextureName() const
{
   return m_texture->GetName();
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
Sprite::SetColor(const glm::vec3& color)
{
   m_currentState.m_color = glm::vec4(color, 1.0f);
}

void
Sprite::SetTextureFromFile(const std::string& filePath)
{
   m_texture = TextureLibrary::GetTexture(filePath);
}

void
Sprite::SetTranslateValue(const glm::vec2& translateBy)
{
   m_currentState.m_currentPosition = m_initialPosition + translateBy;
   m_currentState.m_translateVal = glm::vec3(m_initialPosition + translateBy, 0.0f);
}

void
Sprite::SetInitialPosition(const glm::vec2& globalPosition)
{
   m_initialPosition = globalPosition;
}

Texture&
Sprite::GetTexture()
{
   return *m_texture;
}

void
Sprite::Rotate(float angle, RotationType type)
{
   m_currentState.m_angle = type == RotationType::DEGREES ? glm::degrees(angle) : angle;
   m_currentState.m_angle =
      glm::clamp(m_currentState.m_angle, glm::radians(-360.0f), glm::radians(360.0f));
}

void
Sprite::RotateCumulative(float angle, RotationType type)
{
   m_currentState.m_angle += type == RotationType::DEGREES ? glm::degrees(angle) : angle;
   m_currentState.m_angle =
      glm::clamp(m_currentState.m_angle, glm::radians(-360.0f), glm::radians(360.0f));
}

void
Sprite::Scale(const glm::vec2& scaleValue)
{
   m_currentState.m_scaleVal = scaleValue;
   m_currentState.m_scaleVal.x =
      glm::clamp(m_currentState.m_scaleVal.x, s_SCALERANGE.first, s_SCALERANGE.second);
   m_currentState.m_scaleVal.y =
      glm::clamp(m_currentState.m_scaleVal.y, s_SCALERANGE.first, s_SCALERANGE.second);

   m_size = static_cast< glm::vec2 >(m_size)
            * (m_currentState.m_scaleVal + m_currentState.m_uniformScaleValue);
}

void
Sprite::ScaleCumulative(const glm::vec2& scaleValue)
{
   m_currentState.m_scaleVal += scaleValue;
   m_currentState.m_scaleVal.x =
      glm::clamp(m_currentState.m_scaleVal.x, s_SCALERANGE.first, s_SCALERANGE.second);
   m_currentState.m_scaleVal.y =
      glm::clamp(m_currentState.m_scaleVal.y, s_SCALERANGE.first, s_SCALERANGE.second);
}

void
Sprite::Translate(const glm::vec2& translateValue)
{
   m_currentState.m_currentPosition += translateValue;
   m_currentState.m_translateVal += translateValue;
}

void
Sprite::ScaleUniformly(const float scaleValue)
{
   m_currentState.m_uniformScaleValue = scaleValue;
   /*  m_currentState.m_scaleVal += glm::vec2(scaleValue, scaleValue);
     m_currentState.m_scaleVal.x = glm::clamp(m_currentState.m_scaleVal.x, m_scaleRange.first,
     m_scaleRange.second); m_currentState.m_scaleVal.y = glm::clamp(m_currentState.m_scaleVal.y,
     m_scaleRange.first, m_scaleRange.second);*/
}

std::array< glm::vec2, 4 >
Sprite::GetTransformedRectangle() const
{
   glm::mat4 transformMat =
      glm::translate(glm::mat4(1.0f), glm::vec3(m_currentState.m_translateVal, 0.0f))
      * glm::rotate(glm::mat4(1.0f), m_currentState.m_angle, {0.0f, 0.0f, 1.0f})
      * glm::scale(glm::mat4(1.0f), {m_size, 1.0f});

   const glm::vec2 topLeft = transformMat * glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f);
   const glm::vec2 bottomLeft = transformMat * glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f);
   const glm::vec2 topRight = transformMat * glm::vec4(0.5f, 0.5f, 0.0f, 1.0f);
   const glm::vec2 bottomRight = transformMat * glm::vec4(0.5f, -0.5f, 0.0f, 1.0f);

   return {topRight, topLeft, bottomLeft, bottomRight};
}

} // namespace dgame