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
Camera::Move(const glm::vec3& moveAxes)
{
   const auto xAxesVector = glm::cross(m_lookAtDirection, m_upVector);
   const auto yAxesVector = m_upVector;

   const auto moveBy = (moveAxes.x * xAxesVector) + (moveAxes.y * yAxesVector);

   m_position += moveBy * m_cameraSpeed;

   UpdateViewMatrix();
}

void
Camera::Rotate(float angle)
{
   m_upVector = glm::rotateZ(m_upVector, angle);

   m_rotationValue += angle;

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

void
Camera::UpdateViewMatrix()
{
   m_viewMatrx = glm::lookAt(m_position, m_position + m_lookAtDirection, m_upVector);
}