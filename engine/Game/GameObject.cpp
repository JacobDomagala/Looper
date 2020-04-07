#include <Game.hpp>
#include <GameObject.hpp>
#include <Window.hpp>

GameObject::GameObject(Game& game, const glm::vec2& positionOnMap, const glm::ivec2& size, const std::string& sprite) : m_gameHandle(game)
{
   m_currentState.m_globalPosition = m_gameHandle.GetLevel().GetGlobalVec(positionOnMap);
   m_currentState.m_localPosition = positionOnMap;
   m_currentState.m_visible = true;
   m_collision = m_sprite.SetSpriteTextured(m_currentState.m_globalPosition, size, sprite);
   m_currentState.m_centeredGlobalPosition = m_sprite.GetCenteredPosition();
   m_currentState.m_centeredLocalPosition = m_gameHandle.GetLevel().GetLocalVec(m_currentState.m_centeredGlobalPosition);
}

glm::vec2
GameObject::GetScreenPositionPixels() const
{
   // Get the world coords
   glm::vec4 screenPosition = m_gameHandle.GetProjection() * glm::vec4(m_currentState.m_centeredGlobalPosition, 0.0f, 1.0f);

   // convert from <-1,1> to <0,1>
   glm::vec2 tmpPos = (glm::vec2(screenPosition.x, screenPosition.y) + glm::vec2(1.0f, 1.0f)) / glm::vec2(2.0f, 2.0f);

   // convert from (0,0)->(1,1) [BOTTOM LEFT CORNER] to (0,0)->(WIDTH,HEIGHT) [TOP LEFT CORNER]
   tmpPos.x *= WIDTH;
   tmpPos.y *= -HEIGHT;
   tmpPos.y += HEIGHT;

   return tmpPos;
}

void
GameObject::SetColor(const glm::vec3& color)
{
   m_sprite.SetColor(color);
}

glm::vec2
GameObject::GetSize() const
{
   return m_sprite.GetSize();
}

glm::ivec2
GameObject::GetCenteredLocalPosition() const
{
   return m_currentState.m_centeredLocalPosition;
}

void
GameObject::SetCenteredLocalPosition(const glm::ivec2& pos)
{
   m_currentState.m_centeredLocalPosition = pos;
}

void
GameObject::SetLocalPosition(const glm::ivec2& position)
{
   m_currentState.m_localPosition = position;
}

void
GameObject::SetGlobalPosition(const glm::vec2& position)
{
   m_currentState.m_globalPosition = position;
}

glm::vec2
GameObject::GetGlobalPosition() const
{
   return m_currentState.m_globalPosition;
}

glm::ivec2
GameObject::GetLocalPosition() const
{
   return m_currentState.m_localPosition;
}

glm::vec2
GameObject::GetCenteredGlobalPosition() const
{
   return m_currentState.m_centeredGlobalPosition;
}

void
GameObject::SetShaders(const Shaders& program)
{
   m_program = program;
}

void
GameObject::SetTexture(const Texture& texture)
{
   m_sprite.SetTexture(texture);
}

void
GameObject::CreateSprite(const glm::vec2& position, const glm::ivec2& size)
{
   m_sprite.SetSprite(position, size);
   m_currentState.m_globalPosition = m_sprite.GetCenteredPosition();
}

void
GameObject::CreateSpriteTextured(const glm::vec2& position, const glm::ivec2& size, const std::string& fileName)
{
   m_collision = m_sprite.SetSpriteTextured(position, size, fileName);
   m_currentState.m_globalPosition = m_sprite.GetCenteredPosition();
}

void
GameObject::Move(const glm::vec2& moveBy, bool isCameraMovement)
{
   m_sprite.Translate(moveBy);
   m_currentState.m_globalPosition += moveBy;
   m_currentState.m_centeredGlobalPosition += moveBy;

   if (!isCameraMovement)
   {
      m_currentState.m_localPosition += moveBy;
      m_currentState.m_centeredLocalPosition += moveBy;
   }
}

void
GameObject::Update(bool isReverse)
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

   m_sprite.Update(isReverse);
   UpdateInternal(isReverse);
}

void
GameObject::Render(const glm::mat4& projectionMat, const Shaders& program)
{
   m_sprite.Render(projectionMat, program);
}