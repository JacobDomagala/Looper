#include "game_object.hpp"
#include "application.hpp"
#include "game.hpp"
#include "renderer/camera/collision_camera.hpp"
#include "renderer/window/window.hpp"

namespace looper {

GameObject::GameObject(Application* application, const glm::vec3& position, const glm::vec2& size,
                       const std::string& sprite, ObjectType type)
   : Object(type), appHandle_(application)
{
   auto newPosition = position;
   switch (type)
   {
      case looper::ObjectType::OBJECT: {
         newPosition.z = 0.1f;
      }
      break;

      default:
         break;
   }

   sprite_.SetSpriteTextured(newPosition, size, sprite);
   currentGameObjectState_.position_ = glm::vec2(position);
   currentGameObjectState_.visible_ = true;
   currentGameObjectState_.centeredPosition_ = sprite_.GetPosition();
   currentGameObjectState_.previousPosition_ = glm::vec2(position);


   currentGameObjectState_.nodes_ =
      appHandle_->GetLevel().GetTilesFromBoundingBox(sprite_.GetTransformedRectangle());

   appHandle_->GetLevel().OccupyNodes(id_, currentGameObjectState_.nodes_, hasCollision_);
}

GameObject::GameObject(Application* application, const glm::vec2& position, const glm::vec2& size,
                       const std::string& sprite, ObjectType type)
   : GameObject(application, glm::vec3{position, 0.0f}, size, sprite, type)
{
}

GameObject::~GameObject()
{
   appHandle_->GetLevel().FreeNodes(id_, currentGameObjectState_.nodes_, hasCollision_);
}

void
GameObject::Setup(Application* application, const glm::vec3& position, const glm::vec2& size,
                  const std::string& sprite, ObjectType type)
{
   Object::Setup(type);

   appHandle_ = application;
   auto newPosition = position;
   switch (type)
   {
      case looper::ObjectType::OBJECT: {
         newPosition.z = 0.1f;
      }
      break;

      default:
         break;
   }

   sprite_.SetSpriteTextured(newPosition, size, sprite);
   currentGameObjectState_.position_ = glm::vec2(position);
   currentGameObjectState_.visible_ = true;
   currentGameObjectState_.centeredPosition_ = sprite_.GetPosition();
   currentGameObjectState_.previousPosition_ = glm::vec2(position);


   currentGameObjectState_.nodes_ =
      appHandle_->GetLevel().GetTilesFromBoundingBox(sprite_.GetTransformedRectangle());

   appHandle_->GetLevel().OccupyNodes(id_, currentGameObjectState_.nodes_, hasCollision_);
}


bool
GameObject::CheckIfCollidedScreenPosion(const glm::vec2& screenPosition) const
{
   const renderer::CollisionCamera camera(appHandle_->GetCamera().GetPosition(), this);
   return camera.CheckCollision(appHandle_->ScreenToGlobal(screenPosition));
}

glm::vec2
GameObject::GetScreenPositionPixels() const
{
   return appHandle_->GlobalToScreen(currentGameObjectState_.centeredPosition_);
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

void
GameObject::SetPosition(const glm::vec2& position)
{
   currentGameObjectState_.position_ = position;
}

glm::vec2
GameObject::GetPosition() const
{
   return currentGameObjectState_.position_;
}

glm::vec2
GameObject::GetPreviousPosition() const
{
   return currentGameObjectState_.previousPosition_;
}

glm::vec2
GameObject::GetCenteredPosition() const
{
   return currentGameObjectState_.position_;
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
GameObject::CreateSprite(const glm::vec3& position, const glm::ivec2& size)
{
   sprite_.SetSprite(position, size);
   currentGameObjectState_.position_ = sprite_.GetPosition();
}

void
GameObject::CreateSpriteTextured(const glm::vec3& position, const glm::ivec2& size,
                                 const std::string& fileName)
{
   sprite_.SetSpriteTextured(position, size, fileName);
   currentGameObjectState_.position_ = sprite_.GetPosition();
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

std::string
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
   currentGameObjectState_.previousPosition_ = currentGameObjectState_.position_;
   sprite_.Translate(glm::vec3(moveBy, 0.0f));
   currentGameObjectState_.position_ += moveBy;
   currentGameObjectState_.centeredPosition_ += moveBy;

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
