#include "Camera.hpp"
#include "GameObject.hpp"

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>

namespace dgame {

void
Camera::Create(const glm::vec3& position, const glm::ivec2& windowSize, const glm::vec3& lookAt,
               const glm::vec3& upVec, float cameraSpeed)
{
   m_position = position;
   m_lookAtDirection = lookAt;
   m_upVector = upVec;
   m_cameraSpeed = cameraSpeed;
   m_windowSize = windowSize;

   const auto left = -m_windowSize.x / 2.0f;
   const auto right = m_windowSize.x / 2.0f;
   const auto top = m_windowSize.y / 2.0f;
   const auto bottom = -m_windowSize.y / 2.0f;

   m_viewMatrix = glm::lookAt(m_position, m_position + m_lookAtDirection, m_upVector);
   m_projectionMatrix = glm::ortho(left, right, top, bottom, -1.0f, 10.0f);
   m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
}

void
Camera::SetLevelSize(const glm::vec2& size)
{
   m_levelSize = size;
}

void
Camera::SetProjection(float left, float right, float top, float bottom)
{
   m_projectionMatrix = glm::ortho(left, right, top, bottom, -1.0f, 10.0f);
}

void
Camera::SetCameraAtPosition(const glm::vec3& globalPosition)
{
   m_position = globalPosition;
   UpdateViewMatrix();
}

void
Camera::SetCameraAtPosition(const glm::vec2& globalPosition)
{
   SetCameraAtPosition(glm::vec3(globalPosition, 0.0f));
}

void
Camera::SetCameraAtObject(std::shared_ptr< GameObject > object)
{
   m_position = glm::vec3(object->GetCenteredGlobalPosition(), 0.0f);
   UpdateViewMatrix();
}

void
Camera::Move(const glm::vec3& conventionalVector)
{
   m_position += ConvertToCameraVector(conventionalVector) * m_cameraSpeed;

   m_position = glm::vec3(glm::clamp(m_position.x, -m_levelSize.x, m_levelSize.x),
                          glm::clamp(m_position.y, -m_levelSize.y, m_levelSize.y), 0.0f);

   UpdateViewMatrix();
}

void
Camera::Rotate(float angle, bool cumulative)
{
   m_upVector = glm::rotateZ(m_upVector, angle);

   cumulative ? m_rotationValue += angle : m_rotationValue = angle;

   UpdateViewMatrix();
}

void
Camera::Zoom(float value)
{
   m_zoomScale += value * m_zoomSpeed;

   m_zoomScale = std::clamp(m_zoomScale, m_maxZoomOut, m_maxZoomIn);

   const auto left = -m_windowSize.x / (2.0f + m_zoomScale);
   const auto right = m_windowSize.x / (2.0f + m_zoomScale);
   const auto top = m_windowSize.y / (2.0f + m_zoomScale);
   const auto bottom = -m_windowSize.y / (2.0f + m_zoomScale);
   const auto nearPlane = -1.0f;
   const auto farPlane = 1.0f;

   m_projectionMatrix = glm::ortho(left, right, top, bottom, nearPlane, farPlane);
   m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
}

float
Camera::GetZoomLevel() const
{
   return m_zoomScale;
}

const glm::mat4&
Camera::GetViewMatrix() const
{
   return m_viewMatrix;
}

const glm::mat4&
Camera::GetProjectionMatrix() const
{
   return m_projectionMatrix;
}

const glm::mat4&
Camera::GetViewProjectionMatrix() const
{
   return m_viewProjectionMatrix;
}

const glm::vec3&
Camera::GetPosition() const
{
   return m_position;
}

float
Camera::GetRotation()
{
   return m_rotationValue;
}

glm::vec3
Camera::ConvertToCameraVector(const glm::vec3& conventionalVector) const
{
   const auto xAxesVector = glm::cross(m_lookAtDirection, m_upVector);
   const auto yAxesVector = m_upVector;

   return conventionalVector.x * xAxesVector + conventionalVector.y * yAxesVector;
}

void
Camera::UpdateViewMatrix()
{
   m_viewMatrix = glm::lookAt(m_position, m_position + m_lookAtDirection, m_upVector);
   m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
}

} // namespace dgame