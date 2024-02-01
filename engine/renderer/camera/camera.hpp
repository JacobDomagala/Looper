#pragma once

#include "logger.hpp"

#include <glm/glm.hpp>
#include <memory>

namespace looper::renderer {

class Camera
{
 public:
   static constexpr inline glm::vec3 originalLookAt = {0.0f, 0.0f, -1.0f};
   static constexpr inline glm::vec3 originalUpVec = {0.0f, 1.0f, 0.0f};

 public:
   Camera() = default;

   void
   Create(const glm::vec3& position, const glm::ivec2& windowSize,
          const glm::vec3& lookAt = originalLookAt, const glm::vec3& upVec = originalUpVec,
          float cameraSpeed = 5.0f);

   void
   SetLevelSize(const glm::vec2& size);

   void
   SetProjection(float left, float right, float top, float bottom);

   void
   SetCameraAtPosition(const glm::vec3& globalPosition);

   void
   SetCameraAtPosition(const glm::vec2& globalPosition);

   void
   Move(const glm::vec3& conventionalVector);

   void
   Rotate(float angle, bool cumulative = true);

   void
   Zoom(float value);

   [[nodiscard]] float
   GetZoomLevel() const;

   [[nodiscard]] const glm::mat4&
   GetViewMatrix() const;

   [[nodiscard]] const glm::mat4&
   GetProjectionMatrix() const;

   [[nodiscard]] const glm::mat4&
   GetViewProjectionMatrix() const;

   [[nodiscard]] const glm::vec3&
   GetPosition() const;

   [[nodiscard]] float
   GetRotation() const;

   // Convert 'conventionalVector' to camera state related vector
   // Example:
   // - conventionalVector = vec3(1.0f, 0.0f, 0.0f)
   // - current camera is rotated by 90 degrees
   // function will return vec3(0.0f, 1.0f, 0.0f)
   [[nodiscard]] glm::vec3
   ConvertToCameraVector(const glm::vec3& conventionalVector) const;

 public:
   glm::mat4 viewMatrix_ = {};
   glm::mat4 projectionMatrix_ = {};
   glm::mat4 projectionWithoutZoom_ = {};

 private:
   void
   UpdateViewMatrix();

   float cameraSpeed_ = 0.0f;
   float rotationValue_ = 0.0f;
   float zoomSpeed_ = 0.10f;
   float zoomScale_ = 0.0f;
   float maxZoomIn_ = 2.5f;
   float maxZoomOut_ = -1.9f;

   const float nearPlane_ = 1.0f;
   const float farPlane_ = -1.0f;

   glm::vec2 levelSize_ = {};
   glm::vec2 windowSize_ = {};

   glm::vec3 position_ = {};
   glm::vec3 upVector_ = {};
   glm::vec3 lookAtDirection_ = {};
};

} // namespace looper::renderer