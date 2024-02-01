#include "camera.hpp"
#include "game_object.hpp"

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>

namespace looper::renderer {

void
Camera::Create(const glm::vec3& position, const glm::ivec2& windowSize, const glm::vec3& lookAt,
               const glm::vec3& upVec, float cameraSpeed)
{
   position_ = position;
   lookAtDirection_ = lookAt;
   upVector_ = upVec;
   cameraSpeed_ = cameraSpeed;
   windowSize_ = windowSize;

   const auto left = -windowSize_.x / 2.0f;
   const auto right = windowSize_.x / 2.0f;
   const auto top = windowSize_.y / 2.0f;
   const auto bottom = -windowSize_.y / 2.0f;

   viewMatrix_ = glm::lookAt(position_, position_ + lookAtDirection_, upVector_);

   // NOLINTNEXTLINE top and bottom swapped intentionally
   projectionMatrix_ = glm::ortho(left, right, top, bottom, nearPlane_, farPlane_);
   projectionWithoutZoom_ = projectionMatrix_;
}

void
Camera::SetLevelSize(const glm::vec2& size)
{
   levelSize_ = size;
}

void
Camera::SetProjection(float left, float right, float top, float bottom)
{
   // NOLINTNEXTLINE top and bottom swapped intentionally
   projectionMatrix_ = glm::ortho(left, right, top, bottom, nearPlane_, farPlane_);
}

void
Camera::SetCameraAtPosition(const glm::vec3& globalPosition)
{
   position_ = globalPosition;
   UpdateViewMatrix();
}

void
Camera::SetCameraAtPosition(const glm::vec2& globalPosition)
{
   SetCameraAtPosition(glm::vec3(globalPosition, 0.0f));
}

void
Camera::Move(const glm::vec3& conventionalVector)
{
   position_ += ConvertToCameraVector(conventionalVector) * cameraSpeed_;

   position_ = glm::vec3(glm::clamp(position_.x, -levelSize_.x, levelSize_.x),
                         glm::clamp(position_.y, -levelSize_.y, levelSize_.y), 0.0f);

   UpdateViewMatrix();
}

void
Camera::Rotate(float angle, bool cumulative)
{
   if (cumulative)
   {
      rotationValue_ += angle;
   }
   else
   {
      upVector_ = glm::rotateZ(upVector_, -rotationValue_);
      UpdateViewMatrix();

      rotationValue_ = angle;
   }

   upVector_ = glm::rotateZ(upVector_, angle);

   UpdateViewMatrix();
}

void
Camera::Zoom(float value)
{
   const auto oldZoom = zoomScale_;
   zoomScale_ = glm::clamp(zoomScale_ + (value * zoomSpeed_), maxZoomOut_, maxZoomIn_);

   if (oldZoom != zoomScale_)
   {
      const auto left = -windowSize_.x / (2.0f + zoomScale_);
      const auto right = windowSize_.x / (2.0f + zoomScale_);
      const auto top = windowSize_.y / (2.0f + zoomScale_);
      const auto bottom = -windowSize_.y / (2.0f + zoomScale_);

      // NOLINTNEXTLINE top and bottom swapped intentionally
      projectionMatrix_ = glm::ortho(left, right, top, bottom, nearPlane_, farPlane_);
   }
}

float
Camera::GetZoomLevel() const
{
   return zoomScale_;
}

const glm::mat4&
Camera::GetViewMatrix() const
{
   return viewMatrix_;
}

const glm::mat4&
Camera::GetProjectionMatrix() const
{
   return projectionMatrix_;
}

const glm::vec3&
Camera::GetPosition() const
{
   return position_;
}

float
Camera::GetRotation() const
{
   return rotationValue_;
}

glm::vec3
Camera::ConvertToCameraVector(const glm::vec3& conventionalVector) const
{
   const auto xAxesVector = glm::cross(lookAtDirection_, upVector_);
   const auto yAxesVector = upVector_;

   return conventionalVector.x * xAxesVector + conventionalVector.y * -yAxesVector;
}

void
Camera::UpdateViewMatrix()
{
   viewMatrix_ = glm::lookAt(position_, position_ + lookAtDirection_, upVector_);
}

} // namespace looper::renderer