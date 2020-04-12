#include "Camera.hpp"
#include "GameObject.hpp"

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
Camera::Move(const glm::vec3& moveBy)
{
   m_position += moveBy * m_cameraSpeed;

   UpdateViewMatrix();
}

void
Camera::Rotate(float angle)
{
   UpdateViewMatrix();
}

const glm::mat4&
Camera::GetViewMatrix() const
{
   return m_viewMatrx;
}

void
Camera::UpdateViewMatrix()
{
   m_viewMatrx = glm::lookAt(m_position, m_position + m_lookAtDirection, m_upVector);
}