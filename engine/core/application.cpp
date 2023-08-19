#include "application.hpp"
#include "renderer/renderer.hpp"
#include "renderer/window/window.hpp"

#include <glm/gtx/rotate_vector.hpp>

namespace looper {

std::shared_ptr< Player >
Application::GetPlayer()
{
   return m_currentLevel->GetPlayer();
}

renderer::Window&
Application::GetWindow()
{
   return *m_window;
}

Level&
Application::GetLevel()
{
   return *m_currentLevel;
}

renderer::Camera&
Application::GetCamera()
{
   return m_camera;
}

time::milliseconds
Application::GetDeltaTime() const
{
   return m_deltaTime;
}

bool
Application::IsGame() const
{
   return m_isGame;
}

void
Application::CenterCameraOnPlayer()
{
   m_camera.SetCameraAtObject(m_currentLevel->GetPlayer());
}

glm::vec2
Application::GlobalToScreen(const glm::vec2& globalPos) const
{
   // convert to <-1, 1>
   const glm::vec2 projectedPosition =
      GetProjection() * GetViewMatrix() * glm::vec4(globalPos, 0.0f, 1.0f);

   // convert to <0, 1>
   auto returnPos = (projectedPosition + glm::vec2(1.0f, 1.0f)) / 2.0f;

   return returnPos * GetWindowSize();
}

glm::vec2
Application::ScreenToGlobal(const glm::vec2& screenPos) const
{
   const auto windowCenterScreen = GetWindowSize() / 2.0f;
   const auto zoomRatio = (GetWindowSize() / (2.0f + GetZoomLevel())) / windowCenterScreen;

   // Compute distance from center of screen to 'screenPos' value
   // Remember to multiply it by the current zoomRatio
   const auto distanceToObject =
      (screenPos - windowCenterScreen) * zoomRatio;

   // Rotate vector according to current camera's rotation
   const auto rotatedDistanceToObject =
      glm::rotateZ(glm::vec3(distanceToObject.x, -distanceToObject.y, 0.0f), m_camera.GetRotation());

   // Compute global position by adding computed distance to camera position (which is located in
   // the center of the screen)
   const auto globalPos = m_camera.GetPosition() + rotatedDistanceToObject;

   return globalPos;
}

void
Application::WindowFocusCallback(WindowFocusEvent& event)
{
   renderer::VulkanRenderer::GetRenderData().windowFocus_ = event.focus_;
   windowInFocus_ = event.focus_;
   event.handled_ = true;
}

} // namespace looper
