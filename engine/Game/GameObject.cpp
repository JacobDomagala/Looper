#include "GameObject.hpp"
#include "Application.hpp"
#include "Game.hpp"
#include "Window.hpp"

namespace dgame {

GameObject::GameObject(Application& contextHandle, const glm::vec2& positionOnMap,
                       const glm::ivec2& size, const std::string& sprite, Object::TYPE type)
   : Object(type), m_appHandle(contextHandle)
{
   m_currentState.m_globalPosition = m_appHandle.GetLevel().GetGlobalVec(positionOnMap);
   m_currentState.m_localPosition = positionOnMap;
   m_currentState.m_visible = true;
   m_collision = m_sprite.SetSpriteTextured(m_currentState.m_globalPosition, size, sprite);
   m_currentState.m_centeredGlobalPosition = m_sprite.GetPosition();
   m_currentState.m_centeredLocalPosition =
      m_appHandle.GetLevel().GetLocalVec(m_currentState.m_centeredGlobalPosition);
   m_type = type;

   m_id = s_currentID;
   ++s_currentID;

   UpdateCollision();
}

bool
GameObject::CheckIfCollidedScreenPosion(const glm::vec2& screenPosition) const
{
   bool collided = false;

   Camera camera = m_appHandle.GetCamera();
   camera.Rotate(m_sprite.GetRotation(), false);

   const auto boundingRectangle = m_sprite.GetTransformedRectangle();

   const auto transformed0 = camera.GetViewMatrix() * glm::vec4(boundingRectangle[0], 0.0f, 1.0f);
   const auto transformed1 = camera.GetViewMatrix() * glm::vec4(boundingRectangle[1], 0.0f, 1.0f);
   // const auto transformed2 = camera.GetViewMatrix() * glm::vec4(boundingRectangle[2],
   // 0.0f, 1.0f);
   const auto transformed3 = camera.GetViewMatrix() * glm::vec4(boundingRectangle[3], 0.0f, 1.0f);

   const auto minX = transformed1.x;
   const auto maxX = transformed0.x;
   const auto minY = transformed3.y;
   const auto maxY = transformed0.y;

   const auto globalPosition =
      camera.GetViewMatrix() * glm::vec4(m_appHandle.ScreenToGlobal(screenPosition), 0.0f, 1.0f);

   // If 'screenPosition' is inside 'object' sprite (rectangle)
   if (globalPosition.x >= minX && globalPosition.x <= maxX && globalPosition.y <= maxY
       && globalPosition.y >= minY)
   {
      collided = true;
   }

   return collided;
}

glm::vec2
GameObject::GetScreenPositionPixels() const
{
   return m_appHandle.GlobalToScreen(m_currentState.m_centeredGlobalPosition);
}

bool
GameObject::Visible() const
{
   return m_currentState.m_visible;
}

void
GameObject::SetSize(const glm::vec2& newSize)
{
   m_sprite.SetSize(newSize);

   m_currentState.m_occupiedNodes = m_appHandle.GetLevel().GameObjectMoved(
      m_sprite.GetTransformedRectangle(), m_currentState.m_occupiedNodes);
}

void
GameObject::SetColor(const glm::vec3& color)
{
   m_sprite.SetColor(color);
}

glm::ivec2
GameObject::GetSize() const
{
   return m_sprite.GetSize();
}

glm::ivec2
GameObject::GetCenteredLocalPosition() const
{
   return m_currentState.m_localPosition;
   // return m_currentState.m_centeredLocalPosition;
}

void
GameObject::SetCenteredLocalPosition(const glm::ivec2& pos)
{
   m_currentState.m_centeredLocalPosition = pos;
}

void
GameObject::SetLocalPosition(const glm::ivec2& position)
{
   assert(position.x >= 0 && position.y >= 0);
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
   return m_currentState.m_globalPosition;
   // return m_currentState.m_centeredGlobalPosition;
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
GameObject::SetShaders(const std::string& shader)
{
   m_shaderName = shader;
}

void
GameObject::CreateSprite(const glm::vec2& position, const glm::ivec2& size)
{
   m_sprite.SetSprite(position, size);
   m_currentState.m_globalPosition = m_sprite.GetPosition();
}

void
GameObject::CreateSpriteTextured(const glm::vec2& position, const glm::ivec2& size,
                                 const std::string& fileName)
{
   m_collision = m_sprite.SetSpriteTextured(position, size, fileName);
   m_currentState.m_globalPosition = m_sprite.GetPosition();
}

void
GameObject::SetHasCollision(bool hasCollision)
{
   m_hasCollision = hasCollision;

   if (!m_hasCollision)
   {
      for (auto& node : m_currentState.m_occupiedNodes)
      {
         m_appHandle.GetLevel().GetPathfinder().SetNodeFreed(node);
      }
   }
   else
   {
      UpdateCollision();
   }
}

bool
GameObject::GetHasCollision() const
{
   return m_hasCollision;
}

void
GameObject::SetName(const std::string& name)
{
   m_name = name;
}

std::string
GameObject::GetName() const
{
   return m_name;
}

void
GameObject::UpdateCollision()
{
   if (m_hasCollision)
   {
      m_currentState.m_occupiedNodes = m_appHandle.GetLevel().GameObjectMoved(
         m_sprite.GetTransformedRectangle(), m_currentState.m_occupiedNodes);
   }
}

std::vector< std::pair< int32_t, int32_t > >
GameObject::GetOccupiedNodes() const
{
   return m_currentState.m_occupiedNodes;
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

   UpdateCollision();
}

void
GameObject::Scale(const glm::vec2& scaleVal, bool cumulative)
{
   cumulative ? m_sprite.ScaleCumulative(scaleVal) : m_sprite.Scale(scaleVal);

   UpdateCollision();
}

void
GameObject::Rotate(float angle, bool cumulative)
{
   cumulative ? m_sprite.RotateCumulative(angle) : m_sprite.Rotate(angle);

   UpdateCollision();
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
GameObject::Render()
{
   m_sprite.Render();
}

Game*
GameObject::ConvertToGameHandle()
{
   auto gameHandle = static_cast< Game* >(&m_appHandle);
   if (gameHandle == nullptr)
   {
      m_appHandle.Log(Logger::TYPE::FATAL, "Game logic called not from Game class");
   }

   return gameHandle;
}

} // namespace dgame
