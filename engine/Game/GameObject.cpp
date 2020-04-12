#include <Context.hpp>
#include <Game.hpp>
#include <GameObject.hpp>
#include <Window.hpp>

GameObject::GameObject(Context& game, const glm::vec2& positionOnMap, const glm::ivec2& size, const std::string& sprite)
   : m_contextHandle(game)
{
   m_currentState.m_globalPosition = m_contextHandle.GetLevel().GetGlobalVec(positionOnMap);
   m_currentState.m_localPosition = positionOnMap;
   m_currentState.m_visible = true;
   m_collision = m_sprite.SetSpriteTextured(m_currentState.m_globalPosition, size, sprite);
   m_currentState.m_centeredGlobalPosition = m_sprite.GetCenteredPosition();
   m_currentState.m_centeredLocalPosition = m_contextHandle.GetLevel().GetLocalVec(m_currentState.m_centeredGlobalPosition);
}

bool
GameObject::CheckIfCollidedScreenPosion(const glm::vec2& screenPosition) const
{
   bool collided = false;

   const auto halfSize = GetSize() / 2.0f;

   const auto minBound = m_contextHandle.GlobalToScreen(m_currentState.m_centeredGlobalPosition - halfSize);
   const auto maxBound = m_contextHandle.GlobalToScreen(m_currentState.m_centeredGlobalPosition + halfSize);

   const auto objectLeftSize = minBound.x;
   const auto objectRightSize = maxBound.x;
   const auto objectTopSize = minBound.y;
   const auto objectBottomSize = maxBound.y;

   // If 'screenPosition' is inside 'object' sprite (rectangle)
   if (screenPosition.x >= objectLeftSize && screenPosition.x <= objectRightSize && screenPosition.y <= objectBottomSize
       && screenPosition.y >= objectTopSize)
   {
      collided = true;
   }

   return collided;
}

glm::vec2
GameObject::GetScreenPositionPixels() const
{
   return m_contextHandle.GlobalToScreen(m_currentState.m_centeredGlobalPosition);
}

bool
GameObject::Visible() const
{
   return m_currentState.m_visible;
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

const Sprite&
GameObject::GetSprite() const
{
   return m_sprite;
}

Sprite&
GameObject::GetSprite()
{
   return m_sprite;
}

void
GameObject::SetShaders(const Shaders& program)
{
   m_program = program;
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
GameObject::SetObjectSelected()
{
   SetColor({1.0f, 0.0f, 0.0f});
}

void
GameObject::SetObjectUnselected()
{
   SetColor({1.0f, 1.0f, 1.0f});
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
GameObject::Scale(const glm::vec2& scaleVal)
{
   m_sprite.Scale(scaleVal);
}

void
GameObject::Rotate(float angle, bool cumulative)
{
   cumulative ? m_sprite.RotateCumulative(angle) : m_sprite.Rotate(angle);
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
GameObject::Render(Shaders& program)
{
   m_sprite.Render(m_contextHandle, program);
}

Game*
GameObject::ConvertToGameHandle()
{
   auto gameHandle = static_cast< Game* >(&m_contextHandle);
   if (gameHandle == nullptr)
   {
      m_contextHandle.Log(Logger::TYPE::FATAL, "Game logic called not from Game class");
   }

   return gameHandle;
}