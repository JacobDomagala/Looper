#include "game_object.hpp"
#include "application.hpp"
#include "game.hpp"
#include "renderer/camera/collision_camera.hpp"
#include "renderer/window/window.hpp"

namespace looper {

GameObject::GameObject(Application& application, const glm::vec2& position, const glm::ivec2& size,
                       const std::string& sprite, Object::TYPE type)
   : Object(type), m_appHandle(application)
{
   m_sprite.SetSpriteTextured(position, size, sprite);
   m_currentGameObjectState.m_position = position;
   m_currentGameObjectState.m_visible = true;
   m_currentGameObjectState.m_centeredPosition = m_sprite.GetPosition();

   UpdateCollision();
}

bool
GameObject::CheckIfCollidedScreenPosion(const glm::vec2& screenPosition) const
{
   renderer::CollisionCamera camera(m_appHandle.GetCamera().GetPosition(), this);
   return camera.CheckCollision(m_appHandle.ScreenToGlobal(screenPosition));
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

const renderer::Sprite&
GameObject::GetSprite() const
{
   return m_sprite;
}

renderer::Sprite&
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
   m_sprite.SetSpriteTextured(position, size, fileName);
   m_currentGameObjectState.m_position = m_sprite.GetPosition();
}

void
GameObject::SetHasCollision(bool hasCollision)
{
   m_hasCollision = hasCollision;

   if (!m_hasCollision)
   {
      for (const auto& node : m_currentGameObjectState.m_occupiedNodes)
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
