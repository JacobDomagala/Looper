#include "GameObject.hpp"
#include "Application.hpp"
#include "Game.hpp"
#include "Window.hpp"

namespace looper {

GameObject::GameObject(Application& application, const glm::vec2& position, const glm::ivec2& size,
                       const std::string& sprite, Object::TYPE type)
   : Object(type),
     m_appHandle(application),
     m_collision(m_sprite.SetSpriteTextured(position, size, sprite))
{
   m_currentGameObjectState.m_position = position;
   m_currentGameObjectState.m_visible = true;
   m_currentGameObjectState.m_centeredPosition = m_sprite.GetPosition();

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
   return m_appHandle.GlobalToScreen(m_currentGameObjectState.m_centeredPosition);
}

bool
GameObject::Visible() const
{
   return m_currentGameObjectState.m_visible;
}

void
GameObject::SetSize(const glm::vec2& newSize)
{
   m_sprite.SetSize(newSize);

   m_currentGameObjectState.m_occupiedNodes = m_appHandle.GetLevel().GameObjectMoved(
      m_sprite.GetTransformedRectangle(), m_currentGameObjectState.m_occupiedNodes, m_id);
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

void
GameObject::SetPosition(const glm::vec2& position)
{
   m_currentGameObjectState.m_position = position;
}

glm::vec2
GameObject::GetPosition() const
{
   return m_currentGameObjectState.m_position;
}

glm::vec2
GameObject::GetCenteredPosition() const
{
   return m_currentGameObjectState.m_position;
   // return m_currentGameObjectState.m_centeredPosition;
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
   m_currentGameObjectState.m_position = m_sprite.GetPosition();
}

void
GameObject::CreateSpriteTextured(const glm::vec2& position, const glm::ivec2& size,
                                 const std::string& fileName)
{
   m_collision = m_sprite.SetSpriteTextured(position, size, fileName);
   m_currentGameObjectState.m_position = m_sprite.GetPosition();
}

void
GameObject::SetHasCollision(bool hasCollision)
{
   m_hasCollision = hasCollision;

   if (!m_hasCollision)
   {
      for (auto& node : m_currentGameObjectState.m_occupiedNodes)
      {
         m_appHandle.GetLevel().GetPathfinder().SetNodeFreed(node, m_id);
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
      m_currentGameObjectState.m_occupiedNodes = m_appHandle.GetLevel().GameObjectMoved(
         m_sprite.GetTransformedRectangle(), m_currentGameObjectState.m_occupiedNodes, m_id);
   }
}

std::vector< Tile_t >
GameObject::GetOccupiedNodes() const
{
   return m_currentGameObjectState.m_occupiedNodes;
}

void
GameObject::Move(const glm::vec2& moveBy)
{
   m_sprite.Translate(moveBy);
   m_currentGameObjectState.m_position += moveBy;
   m_currentGameObjectState.m_centeredPosition += moveBy;

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
      m_currentGameObjectState = m_gameObjectStatesQueue.back();
      m_gameObjectStatesQueue.pop_back();
   }
   else
   {
      m_gameObjectStatesQueue.push_back(m_currentGameObjectState);
      if (m_gameObjectStatesQueue.size() >= NUM_FRAMES_TO_SAVE)
      {
         m_gameObjectStatesQueue.pop_front();
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
   auto* gameHandle = dynamic_cast< Game* >(&m_appHandle);
   if (gameHandle == nullptr)
   {
      Logger::Fatal("GameObject: Game logic called not from Game class");
   }

   return gameHandle;
}

} // namespace looper
