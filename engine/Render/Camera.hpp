#pragma once

#include <glm/glm.hpp>
#include <memory>

class GameObject;

class Camera
{
 public:
   Camera() = default;
   ~Camera() = default;

   void
   Create(const glm::vec3& position, const glm::vec3& lookAt, const glm::vec3& upVec, float cameraSpeed = 5.0f);

   void
   SetCameraAtPosition(const glm::vec3& globalPosition);

   void
   SetCameraAtObject(std::shared_ptr< GameObject > object);

   void
   Move(const glm::vec3& moveBy);

   void
   Rotate(float angle);

   const glm::mat4&
   GetViewMatrix() const;

 private:
   void
   UpdateViewMatrix();

   float m_cameraSpeed = 0.0f;

   glm::vec3 m_position;
   glm::vec3 m_upVector;
   glm::vec3 m_lookAtDirection;
   glm::mat4 m_viewMatrx;
};
