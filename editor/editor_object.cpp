#include "editor_object.hpp"
#include "animatable.hpp"
#include "editor.hpp"
#include "enemy.hpp"

namespace looper {

EditorObject::EditorObject(Editor* editor, const glm::vec2& positionOnMap, const glm::ivec2& size,
                           const std::string& sprite, Object::ID linkedObject)
   : Object(ObjectType::EDITOR_OBJECT),
     editor_(editor),
     position_(positionOnMap),
     centeredPosition_(positionOnMap),
     objectID_(linkedObject),
     hasLinkedObject_(true)
{
   auto depth = 0.01f;

   sprite_.SetSpriteTextured(glm::vec3{position_, depth}, size, sprite);
}

bool
EditorObject::CheckIfCollidedScreenPosion(const glm::vec2& screenPosition) const
{
   bool collided = false;

   renderer::Camera camera = editor_->GetCamera();
   camera.Rotate(sprite_.GetRotation(), false);

   const auto boundingRectangle = sprite_.GetTransformedRectangle();

   const auto transformed0 = camera.GetViewMatrix() * glm::vec4(boundingRectangle[0], 0.0f, 1.0f);
   const auto transformed1 = camera.GetViewMatrix() * glm::vec4(boundingRectangle[1], 0.0f, 1.0f);
   const auto transformed3 = camera.GetViewMatrix() * glm::vec4(boundingRectangle[3], 0.0f, 1.0f);

   const auto minX = transformed1.x;
   const auto maxX = transformed0.x;
   const auto minY = transformed3.y;
   const auto maxY = transformed0.y;

   const auto globalPosition =
      camera.GetViewMatrix() * glm::vec4(editor_->ScreenToGlobal(screenPosition), 0.0f, 1.0f);

   // If 'screenPosition' is inside 'object' sprite (rectangle)
   if (globalPosition.x >= minX && globalPosition.x <= maxX && globalPosition.y <= maxY
       && globalPosition.y >= minY)
   {
      collided = true;
   }

   return collided;
}

glm::vec2
EditorObject::GetScreenPositionPixels() const
{
   return editor_->GlobalToScreen(centeredPosition_);
}

bool
EditorObject::Visible() const
{
   return visible_;
}

void
EditorObject::SetColor(const glm::vec4& color)
{
   sprite_.SetColor(color);
}

void
EditorObject::SetIsBackground(bool isBackground)
{
   isBackground_ = isBackground;
}

bool
EditorObject::GetIsBackground() const
{
   return isBackground_;
}

glm::ivec2
EditorObject::GetSize() const
{
   return sprite_.GetSize();
}

void
EditorObject::SetPosition(const glm::vec2& position)
{
   position_ = position;
}

glm::vec2
EditorObject::GetPosition() const
{
   return position_;
}

glm::vec2
EditorObject::GetCenteredPosition() const
{
   return centeredPosition_;
}

const renderer::Sprite&
EditorObject::GetSprite() const
{
   return sprite_;
}

renderer::Sprite&
EditorObject::GetSprite()
{
   return sprite_;
}

void
EditorObject::CreateSpriteTextured(const glm::vec2& /*position*/, const glm::ivec2& /*size*/,
                                   const std::string& /*fileName*/)
{
   position_ = sprite_.GetPosition();
}

void
EditorObject::SetObjectSelected()
{
   SetColor({1.0f, 0.0f, 0.0f, 1.0f});
   selected_ = true;
}

void
EditorObject::SetObjectUnselected()
{
   selected_ = false;
   SetColor({1.0f, 1.0f, 1.0f, 1.0f});
}

void
EditorObject::SetName(const std::string& name)
{
   name_ = name;
}

std::string
EditorObject::GetName() const
{
   return name_;
}

Object::ID
EditorObject::GetLinkedObjectID() const
{
   return objectID_;
}

void
EditorObject::DeleteLinkedObject()
{
   if (hasLinkedObject_)
   {
      switch (Object::GetTypeFromID(objectID_))
      {
         case ObjectType::ANIMATION_POINT: {
            auto& animationPoint =
               dynamic_cast< AnimationPoint& >(editor_->GetLevel().GetObjectRef(objectID_));
            auto& object = editor_->GetLevel().GetObjectRef(animationPoint.m_parent);

            auto& enemy = dynamic_cast< Enemy& >(object);
            enemy.DeleteAnimationNode(objectID_);
            enemy.ResetAnimation();
            enemy.Move(enemy.GetAnimationStartLocation() - enemy.GetPosition());
         }
         break;

         default: {
         }
      }
   }
}

void
EditorObject::Move(const glm::vec2& moveBy)
{
   sprite_.Translate(glm::vec3(moveBy, 0.0f));
   position_ += moveBy;
   centeredPosition_ += moveBy;

   if (hasLinkedObject_)
   {
      switch (Object::GetTypeFromID(objectID_))
      {
         case ObjectType::ANIMATION_POINT: {
            auto& animationPoint =
               dynamic_cast< AnimationPoint& >(editor_->GetLevel().GetObjectRef(objectID_));
            animationPoint.m_end += moveBy;
            editor_->UpdateAnimationData(animationPoint.m_parent);
         }
         break;

         default: {
         }
      }
   }
}

void
EditorObject::Scale(const glm::vec2& scaleVal, bool cumulative)
{
   cumulative ? sprite_.ScaleCumulative(scaleVal) : sprite_.Scale(scaleVal);
}

void
EditorObject::Rotate(float angle, bool cumulative)
{
   cumulative ? sprite_.RotateCumulative(angle) : sprite_.Rotate(angle);

   auto rotate = sprite_.GetRotation(renderer::RotationType::degrees);
   if (hasLinkedObject_)
   {
      switch (Object::GetTypeFromID(objectID_))
      {
         case ObjectType::ANIMATION_POINT: {
            auto& animationPoint =
               dynamic_cast< AnimationPoint& >(editor_->GetLevel().GetObjectRef(objectID_));
            animationPoint.m_rotation = rotate;
            editor_->UpdateAnimationData(animationPoint.m_parent);
         }
         default: {
         }
      }
   }
}

void
EditorObject::Update(bool /*isReverse*/)
{
}

void
EditorObject::Render()
{
   if (hasLinkedObject_)
   {
      switch (Object::GetTypeFromID(objectID_))
      {
         case ObjectType::ANIMATION_POINT: {
            sprite_.SetColor({1.0f, 1.0f, 1.0f, static_cast< float >(visible_)});
            sprite_.Render();
         }
         break;

         default: {
         }
      }
   }
}

void
EditorObject::SetVisible(bool visible)
{
   visible_ = visible;
   sprite_.SetColor({1.0f, 1.0f, 1.0f, static_cast< float >(visible_)});
}

bool
EditorObject::IsVisible() const
{
   return visible_;
}

} // namespace looper
