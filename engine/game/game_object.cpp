#include "game_object.hpp"
#include "application.hpp"
#include "game.hpp"
#include "renderer/camera/collision_camera.hpp"
#include "renderer/window/window.hpp"

namespace looper {

GameObject::GameObject(Application* application, const glm::vec2& position, const glm::vec2& size,
                       const std::string& sprite, ObjectType type)
   : Object(type), appHandle_(application)
{
   uint32_t renderLayer = 2;
   switch (type)
   {
      case ObjectType::ENEMY:
      case ObjectType::PLAYER: {
         renderLayer = 1;
      }
      break;
      default: {
      }
   }

   sprite_.SetSpriteTextured(position, size, sprite, renderLayer);
   currentGameObjectState_.visible_ = true;
   currentGameObjectState_.previousPosition_ = glm::vec2(position);

   currentGameObjectState_.nodes_ =
      appHandle_->GetLevel().GetTilesFromBoundingBox(sprite_.GetTransformedRectangle());

   appHandle_->GetLevel().OccupyNodes(id_, currentGameObjectState_.nodes_, hasCollision_);
}

GameObject::~GameObject()
{
   appHandle_->GetLevel().FreeNodes(id_, currentGameObjectState_.nodes_, hasCollision_);
}

void
GameObject::Setup(Application* application, const glm::vec2& position, const glm::vec2& size,
                  const std::string& sprite, ObjectType type, uint32_t renderLayer)
{
   Object::Setup(type);

   appHandle_ = application;

   switch (type)
   {
      case ObjectType::ENEMY:
      case ObjectType::PLAYER: {
         renderLayer = 1;
      }
      break;
      default: {
      }
   }

   sprite_.SetSpriteTextured(position, size, sprite, renderLayer);
   currentGameObjectState_.visible_ = true;
   currentGameObjectState_.previousPosition_ = position;


   currentGameObjectState_.nodes_ =
      appHandle_->GetLevel().GetTilesFromBoundingBox(sprite_.GetTransformedRectangle());

   appHandle_->GetLevel().OccupyNodes(id_, currentGameObjectState_.nodes_, hasCollision_);
}


bool
GameObject::CheckIfCollidedScreenPosion(const glm::vec2& screenPosition) const
{
   const renderer::CollisionCamera camera(appHandle_->GetCamera().GetPosition(), &sprite_);
   return camera.CheckCollision(appHandle_->ScreenToGlobal(screenPosition));
}

glm::vec2
GameObject::GetScreenPositionPixels() const
{
   return appHandle_->GlobalToScreen(sprite_.GetPosition());
}

bool
GameObject::Visible() const
{
   return currentGameObjectState_.visible_;
}

void
GameObject::SetSize(const glm::vec2& newSize)
{
   sprite_.SetSize(newSize);
   updateCollision_ = true;
}

void
GameObject::SetColor(const glm::vec4& color)
{
   sprite_.SetColor(color);
}

glm::ivec2
GameObject::GetSize() const
{
   return sprite_.GetSize();
}

glm::vec2
GameObject::GetPosition() const
{
   return sprite_.GetPosition();
}

glm::vec2
GameObject::GetPreviousPosition() const
{
   return currentGameObjectState_.previousPosition_;
}

glm::vec2
GameObject::GetCenteredPosition() const
{
   return sprite_.GetPosition();
   // return currentGameObjectState_.centeredPosition_;
}

const renderer::Sprite&
GameObject::GetSprite() const
{
   return sprite_;
}

renderer::Sprite&
GameObject::GetSprite()
{
   return sprite_;
}

void
GameObject::CreateSpriteTextured(const glm::vec3& position, const glm::ivec2& size,
                                 const std::string& fileName)
{
   sprite_.SetSpriteTextured(position, size, fileName);
}

void
GameObject::SetHasCollision(bool hasCollision)
{
   if (hasCollision == hasCollision_)
   {
      return;
   }

   hasCollision_ = hasCollision;

   if (!hasCollision_)
   {
      auto& pathfinder = appHandle_->GetLevel().GetPathfinder();
      for (const auto& node : currentGameObjectState_.nodes_)
      {
         pathfinder.SetNodeFreed(node, id_);
      }
   }
   else
   {
      appHandle_->GetLevel().OccupyNodes(id_, currentGameObjectState_.nodes_, hasCollision_);
   }
}

bool
GameObject::GetHasCollision() const
{
   return hasCollision_;
}

void
GameObject::SetName(const std::string& name)
{
   name_ = name;
}

const std::string&
GameObject::GetName() const
{
   return name_;
}

void
GameObject::UpdateCollision()
{
   currentGameObjectState_.nodes_ = appHandle_->GetLevel().GameObjectMoved(
      sprite_.GetTransformedRectangle(), currentGameObjectState_.nodes_, id_, hasCollision_);
}

std::vector< Tile >
GameObject::GetOccupiedNodes() const
{
   return currentGameObjectState_.nodes_;
}

void
GameObject::Move(const glm::vec2& moveBy)
{
   currentGameObjectState_.previousPosition_ = sprite_.GetPosition();
   sprite_.Translate(glm::vec3(moveBy, 0.0f));

   updateCollision_ = true;
}

void
GameObject::Scale(const glm::vec2& scaleVal, bool cumulative)
{
   cumulative ? sprite_.ScaleCumulative(scaleVal) : sprite_.Scale(scaleVal);

   updateCollision_ = true;
}

void
GameObject::Rotate(float angle, bool cumulative)
{
   cumulative ? sprite_.RotateCumulative(angle) : sprite_.Rotate(angle);
   updateCollision_ = true;
}

void
GameObject::Update(bool isReverse)
{
   if (isReverse)
   {
      currentGameObjectState_ = gameObjectStatesQueue_.GetLastState();
   }
   else
   {
      gameObjectStatesQueue_.PushState(currentGameObjectState_);
   }

   sprite_.Update(isReverse);
   UpdateInternal(isReverse);
}

void
GameObject::Render()
{
   if (updateCollision_)
   {
      UpdateCollision();
      updateCollision_ = false;
   }

   sprite_.Render();
}

Game*
GameObject::ConvertToGameHandle()
{
   auto* gameHandle = dynamic_cast< Game* >(appHandle_);
   if (gameHandle == nullptr)
   {
      Logger::Fatal("GameObject: Game logic called not from Game class");
   }

   return gameHandle;
}

} // namespace looper
