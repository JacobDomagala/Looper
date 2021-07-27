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
   Create(const glm::vec3& position, const glm::ivec2& windowSize,
          const glm::vec3& lookAt = {0.0f, 0.0f, -1.0f},
          const glm::vec3& upVec = {0.0f, 1.0f, 0.0f}, float cameraSpeed = 5.0f);

   void
   SetLevelSize(const glm::vec2& size);

   void
   SetProjection(float left, float right, float top, float bottom);

   void
   SetCameraAtPosition(const glm::vec3& globalPosition);

   void
   SetCameraAtPosition(const glm::vec2& globalPosition);

   void
   SetCameraAtObject(std::shared_ptr< GameObject > object);

   void
   Move(const glm::vec3& conventionalVector);

   void
   Rotate(float angle, bool cumulative = true);

   void
   Zoom(float value);

   float
   GetZoomLevel() const;

   const glm::mat4&
   GetViewMatrix() const;

   const glm::mat4&
   GetProjectionMatrix() const;

   const glm::mat4&
   GetViewProjectionMatrix() const;

   const glm::vec3&
   GetPosition() const;

   float
   GetRotation();

   // Convert 'conventionalVector' to camera state related vector
   // Example:
   // - conventionalVector = vec3(1.0f, 0.0f, 0.0f)
   // - current camera is rotated by 90 degrees
   // function will return vec3(0.0f, 1.0f, 0.0f)
   glm::vec3
   ConvertToCameraVector(const glm::vec3& conventionalVector) const;

 private:
   void
   UpdateViewMatrix();

   Logger m_logger = Logger("Camera");

   float m_cameraSpeed = 0.0f;
   float m_rotationValue = 0.0f;
   float m_zoomSpeed = 0.10f;
   float m_zoomScale = 0.0f;
   float m_maxZoomIn = 1.5f;
   float m_maxZoomOut = -1.5f;

   glm::vec2 m_levelSize;
   glm::vec2 m_windowSize;

   glm::vec3 m_position;
   glm::vec3 m_upVector;
   glm::vec3 m_lookAtDirection;

   glm::mat4 m_viewMatrix;
   glm::mat4 m_projectionMatrix;
   glm::mat4 m_viewProjectionMatrix;
};

} // namespace dgame