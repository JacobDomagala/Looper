#include "Camera.hpp"
#include "GameObject.hpp"

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>

void
Camera::Create(const glm::vec3& position, const glm::vec3& lookAt, const glm::vec3& upVec, float cameraSpeed)
{
   m_position = position;
   m_lookAtDirection = lookAt;
   m_upVector = upVec;
   m_cameraSpeed = cameraSpeed;

   m_viewMatrx = glm::lookAt(m_position, m_position + m_lookAtDirection, m_upVector);
}

void
Camera::SetLevelSize(const glm::vec2& size)
{
   m_levelSize = size;
}

void
Camera::SetCameraAtPosition(const glm::vec3& globalPosition)
{
   m_position = globalPosition;
   UpdateViewMatrix();
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

   m_position = glm::vec3(glm::clamp(m_position.x, 0.0f, m_levelSize.x), glm::clamp(m_position.y, -m_levelSize.y, 0.0f), 0.0f);

   UpdateViewMatrix();
}

void
Camera::Rotate(float angle, bool cumulative)
{
   m_upVector = glm::rotateZ(m_upVector, angle);

   cumulative ? m_rotationValue += angle : m_rotationValue = angle;

   UpdateViewMatrix();
}

const glm::mat4&
Camera::GetViewMatrix() const
{
   return m_viewMatrx;
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
   m_viewMatrx = glm::lookAt(m_position, m_position + m_lookAtDirection, m_upVector);
}