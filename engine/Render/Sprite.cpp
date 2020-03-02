#include "Sprite.hpp"
#include "Game.hpp"

#include <glm/gtx/transform.hpp>

void
Sprite::SetSprite(const glm::vec2& position, const glm::ivec2& size)
{
   glGenVertexArrays(1, &m_vertexArrayBuffer);
   glGenBuffers(1, &m_vertexBuffer);
   glBindVertexArray(m_vertexArrayBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

   glm::vec4 positions[6] = {glm::vec4(0.0f + size.x, 0.0f, 1.0f, 1.0f),         glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
                             glm::vec4(0.0f, 0.0f - size.y, 0.0f, 0.0f),

                             glm::vec4(0.0f + size.x, 0.0f, 1.0f, 1.0f),         glm::vec4(0.0f, 0.0f - size.y, 0.0f, 0.0f),
                             glm::vec4(0.0f + size.x, 0.0f - size.y, 1.0f, 0.0f)};
   m_centeredPosition.x = position.x + (size.x / 2.0f);
   m_centeredPosition.y = position.y - (size.y / 2.0f);
   m_position = position;
   m_size = size;

   m_translateVal = glm::vec3(position, 0.0f);
   m_angle = 0.0f;
   m_scaleVal = glm::vec2(1.0f, 1.0f);
   m_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

   glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
   glBindVertexArray(0);
}

std::unique_ptr< byte_vec4 >
Sprite::SetSpriteTextured(const glm::vec2& position, const glm::ivec2& size, const std::string& fileName)
{
   std::unique_ptr< byte_vec4 > returnPtr = m_texture.LoadTextureFromFile(fileName);

   glGenVertexArrays(1, &m_vertexArrayBuffer);
   glGenBuffers(1, &m_vertexBuffer);
   glBindVertexArray(m_vertexArrayBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);

   glm::vec4 positions[6] = {glm::vec4(0.0f + size.x, 0.0f, 1.0f, 1.0f),         glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
                             glm::vec4(0.0f, 0.0f - size.y, 0.0f, 0.0f),

                             glm::vec4(0.0f + size.x, 0.0f, 1.0f, 1.0f),         glm::vec4(0.0f, 0.0f - size.y, 0.0f, 0.0f),
                             glm::vec4(0.0f + size.x, 0.0f - size.y, 1.0f, 0.0f)};

   m_centeredPosition.x = position.x + (size.x / 2.0f);
   m_centeredPosition.y = position.y - (size.y / 2.0f);
   m_position = position;
   m_size = size;

   m_translateVal = glm::vec3(position, 0.0f);
   m_angle = 0.0f;
   m_scaleVal = glm::vec2(1.0f, 1.0f);
   m_color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

   glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
   glBindVertexArray(0);

   return returnPtr;
}

void
Sprite::Render(Window& window, const Shaders& program)
{
   program.UseProgram();
   glBindVertexArray(m_vertexArrayBuffer);

   glm::mat4 modelMatrix;

   modelMatrix = glm::translate(modelMatrix, m_translateVal);
   modelMatrix = glm::translate(modelMatrix, glm::vec3((m_size.x / 2.0f) * m_scaleVal.x, (m_size.y / -2.0f) * m_scaleVal.y, 0.0f));
   modelMatrix = glm::rotate(modelMatrix, glm::radians(m_angle), glm::vec3(0.0f, 0.0f, 1.0f));
   modelMatrix = glm::translate(modelMatrix, glm::vec3((m_size.x / -2.0f) * m_scaleVal.x, (m_size.y / 2.0f) * m_scaleVal.y, 0.0f));
   modelMatrix = glm::scale(modelMatrix, glm::vec3(m_scaleVal, 1.0f));

   m_texture.Use(program.GetProgram());
   program.SetUniformFloatVec4(m_color, "color");
   program.SetUniformFloatMat4(window.GetProjection(), "projectionMatrix");
   program.SetUniformFloatMat4(modelMatrix, "modelMatrix");

   glDrawArrays(GL_TRIANGLES, 0, 6);
   glBindVertexArray(0);
}
