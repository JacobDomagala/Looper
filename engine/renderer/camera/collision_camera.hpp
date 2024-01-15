#pragma once

#include <glm/glm.hpp>

namespace looper::renderer {
class Sprite;

class CollisionCamera
{
 public:
   CollisionCamera(const glm::vec3& position, const Sprite* obj);

   [[nodiscard]] bool
   CheckCollision(const glm::vec2& globalVec) const;

 private:
   void
   UpdateViewMatrix();

   void
   Rotate(float angle);

   glm::vec3 position_ = {};
   glm::vec3 upVector_ = {};
   glm::vec3 lookAtDirection_ = {};

   glm::mat4 viewMatrix_ = {};
   const Sprite* sprite_ = nullptr;
};

} // namespace looper::renderer
