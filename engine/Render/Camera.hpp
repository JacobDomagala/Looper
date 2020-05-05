#pragma once

#include "Logger.hpp"

#include <glm/glm.hpp>
#include <memory>

namespace dgame {

class GameObject;

class Camera
{
 public:
   Camera() = default;
   ~Camera() = default;

   void
   Create(const glm::vec3& position, const glm::vec3& lookAt, const glm::vec3& upVec, float cameraSpeed = 5.0f);

   void
   SetLevelSize(const glm::vec2& size);

   void
   SetCameraAtPosition(const glm::vec3& globalPosition);

   void
   SetCameraAtObject(std::shared_ptr< GameObject > object);

   void
   Move(const glm::vec3& conventionalVector);

   void
   Rotate(float angle, bool cumulative = true);

   const glm::mat4&
   GetViewMatrix() const;

   const glm::vec3&
   GetPosition() const;

   float
   GetRotation();

   // Convert 'conventionalVector' to camera state related vector
   // Example:
   // - conventionalVector = vec(1.0f, 0.0f, 0.0f)
   // - current camera is rotated by 90 degrees
   // function will return vec2(0.0f, 1.0f, 0.0f)
   glm::vec3
   ConvertToCameraVector(const glm::vec3& conventionalVector) const;

 private:
   void
   UpdateViewMatrix();

   Logger m_logger = Logger("Camera");

   float m_cameraSpeed = 0.0f;
   float m_rotationValue = 0.0f;

   glm::vec2 m_levelSize;

   glm::vec3 m_position;
   glm::vec3 m_upVector;
   glm::vec3 m_lookAtDirection;
   glm::mat4 m_viewMatrx;
};

} // namespace dgame