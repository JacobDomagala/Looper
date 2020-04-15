#include "Sprite.hpp"
#include "Context.hpp"

#include <glm/gtx/transform.hpp>

void
Sprite::SetSprite(const glm::vec2& position, const glm::ivec2& size)
{
   m_texture.CreateColorTexture(size, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

   glGenVertexArrays(1, &m_vertexArrayBuffer);
   glGenBuffers(1, &m_vertexBuffer);
   glBindVertexArray(m_vertexArrayBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

   glm::vec4 positions[] = {glm::vec4(0.0f + size.x, 0.0f, 1.0f, 1.0f),         glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
                            glm::vec4(0.0f, 0.0f - size.y, 0.0f, 0.0f),

                            glm::vec4(0.0f + size.x, 0.0f, 1.0f, 1.0f),         glm::vec4(0.0f, 0.0f - size.y, 0.0f, 0.0f),
                            glm::vec4(0.0f + size.x, 0.0f - size.y, 1.0f, 0.0f)};
   m_centeredPosition.x = position.x + (size.x / 2.0f);
   m_centeredPosition.y = position.y - (size.y / 2.0f);
   m_currentState.m_position = position;
   m_size = size;

   m_currentState.m_translateVal = glm::vec3(position, 0.0f);
   m_currentState.m_angle = 0.0f;
   m_currentState.m_scaleVal = glm::vec2(1.0f, 1.0f);
   m_currentState.m_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

   glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
   glBindVertexArray(0);
}

byte_vec4*
Sprite::SetSpriteTextured(const glm::vec2& position, const glm::ivec2& size, const std::string& fileName)
{
   auto returnPtr = m_texture.LoadTextureFromFile(fileName);

   glGenVertexArrays(1, &m_vertexArrayBuffer);
   glGenBuffers(1, &m_vertexBuffer);
   glBindVertexArray(m_vertexArrayBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

   glm::vec4 positions[] = {glm::vec4(0.0f + size.x, 0.0f, 1.0f, 1.0f),         glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
                            glm::vec4(0.0f, 0.0f - size.y, 0.0f, 0.0f),

                            glm::vec4(0.0f + size.x, 0.0f, 1.0f, 1.0f),         glm::vec4(0.0f, 0.0f - size.y, 0.0f, 0.0f),
                            glm::vec4(0.0f + size.x, 0.0f - size.y, 1.0f, 0.0f)};

   m_centeredPosition.x = position.x + (size.x / 2.0f);
   m_centeredPosition.y = position.y - (size.y / 2.0f);
   m_currentState.m_position = position;
   m_size = size;

   m_currentState.m_translateVal = glm::vec3(position, 0.0f);
   m_currentState.m_angle = 0.0f;
   m_currentState.m_scaleVal = glm::vec2(1.0f, 1.0f);
   m_currentState.m_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

   glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
   glBindVertexArray(0);

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
Sprite::Render(Context& context, Shaders& program)
{
   program.UseProgram();
   glBindVertexArray(m_vertexArrayBuffer);

   glm::mat4 modelMatrix = glm::mat4(1.0f);

   // All transformations are done in reverse:
   // 1. Scale
   // Before rotation transalte
   // 2. Rotate
   // After rotation translate
   // 3. Transalte

   modelMatrix = glm::translate(modelMatrix, m_currentState.m_translateVal);

   // move the sprite back to its original position
   modelMatrix = glm::translate(
      modelMatrix, glm::vec3((m_size.x / 2.0f) * m_currentState.m_scaleVal.x, (m_size.y / -2.0f) * m_currentState.m_scaleVal.y, 0.0f));

   modelMatrix = glm::rotate(modelMatrix, m_currentState.m_angle, glm::vec3(0.0f, 0.0f, 1.0f));

   // move the sprite so it will be rotated around its center
   modelMatrix = glm::translate(
      modelMatrix, glm::vec3((m_size.x / -2.0f) * m_currentState.m_scaleVal.x, (m_size.y / 2.0f) * m_currentState.m_scaleVal.y, 0.0f));

   modelMatrix = glm::scale(modelMatrix, glm::vec3(m_currentState.m_scaleVal, 1.0f));

   m_texture.Use(program.GetProgram());
   program.SetUniformFloatVec4(m_currentState.m_color, "color");
   program.SetUniformFloatVec2(GetSize(), "objectSize");
   program.SetUniformFloatMat4(context.GetProjection(), "projectionMatrix");
   program.SetUniformFloatMat4(context.GetViewMatrix(), "viewMatrix");
   program.SetUniformFloatMat4(modelMatrix, "modelMatrix");

   glDrawArrays(GL_TRIANGLES, 0, 6);
   glBindVertexArray(0);
}


glm::vec2
Sprite::GetCenteredPosition() const
{
   return m_centeredPosition;
}

glm::vec2
Sprite::GetPosition() const
{
   return m_currentState.m_position;
}

glm::ivec2
Sprite::GetSize() const
{
   return m_size;
}

std::string
Sprite::GetTextureName() const
{
   return m_texture.GetName();
}
void
Sprite::SetColor(const glm::vec3& color)
{
   m_currentState.m_color = glm::vec4(color, 1.0f);
}

void
Sprite::SetTextureFromFile(const std::string& filePath)
{
   m_texture.LoadTextureFromFile(filePath);
}

void
Sprite::Rotate(float angle)
{
   m_currentState.m_angle = angle;
}

void
Sprite::RotateCumulative(float angle)
{

   m_currentState.m_angle += angle;
}

void
Sprite::Scale(const glm::vec2& scaleValue)
{
   m_currentState.m_scaleVal += scaleValue;
}

void
Sprite::Translate(const glm::vec2& translateValue)
{
   m_currentState.m_position += translateValue;
   m_currentState.m_translateVal += glm::vec3(translateValue, 0.0f);
}