#pragma once

#include <glm/glm.hpp>

namespace looper {
class GameObject;
}

namespace looper::renderer {

class CollisionCamera
{
 public:
   CollisionCamera(const glm::vec3& position, const GameObject* obj);

   bool
   CheckCollision(const glm::vec2& globalVec);

 private:
   void
   UpdateViewMatrix();

   void
   Rotate(float angle);

   glm::vec3 position_ = {};
   glm::vec3 upVector_ = {};
   glm::vec3 lookAtDirection_ = {};

   glm::mat4 viewMatrix_ = {};
   const GameObject* object_ = nullptr;
};

} // namespace looper::renderer
