#include "collision_camera.hpp"
#include "camera.hpp"
#include "sprite.hpp"

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>

namespace looper::renderer {

CollisionCamera::CollisionCamera(const glm::vec3& position, const Sprite* sprite)
   : position_(position),
     upVector_(Camera::originalUpVec),
     lookAtDirection_(Camera::originalLookAt),
     sprite_(sprite)
{
   Rotate(sprite_->GetRotation());
}

bool
CollisionCamera::CheckCollision(const glm::vec2& globalVec) const
{
   bool collided = false;

   const auto boundingRectangle = sprite_->GetTransformedRectangle();

   const auto transformed0 = viewMatrix_ * glm::vec4(boundingRectangle[0], 0.0f, 1.0f);
   const auto transformed1 = viewMatrix_ * glm::vec4(boundingRectangle[1], 0.0f, 1.0f);
   const auto transformed3 = viewMatrix_ * glm::vec4(boundingRectangle[3], 0.0f, 1.0f);

   const auto minX = transformed1.x;
   const auto maxX = transformed0.x;
   const auto minY = transformed3.y;
   const auto maxY = transformed0.y;

   const auto globalPosition = viewMatrix_ * glm::vec4(globalVec, 0.0f, 1.0f);

   // If 'screenPosition' is inside 'object' sprite (rectangle)
   if (globalPosition.x >= minX && globalPosition.x <= maxX && globalPosition.y <= maxY
       && globalPosition.y >= minY)
   {
      collided = true;
   }

   return collided;
}

void
CollisionCamera::Rotate(float angle)
{
   upVector_ = glm::rotateZ(upVector_, angle);

   UpdateViewMatrix();
}

void
CollisionCamera::UpdateViewMatrix()
{
   viewMatrix_ = glm::lookAt(position_, position_ + lookAtDirection_, upVector_);
}

} // namespace looper::renderer
