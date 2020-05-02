#include "Application.hpp"

#include <GLFW/glfw3.h>
#include <glm/gtx/rotate_vector.hpp>

std::shared_ptr< Player >
Application::GetPlayer()
{
   return m_currentLevel->GetPlayer();
}

Level&
Application::GetLevel()
{
   return *m_currentLevel;
}

Camera&
Application::GetCamera()
{
   return m_camera;
}

Timer::milliseconds
Application::GetDeltaTime()
{
   return m_deltaTime;
}

bool
Application::IsGame()
{
   return m_isGame;
}

void
Application::Log(Logger::TYPE t, const std::string& log)
{
   m_logger.Log(t, log);
}

void
Application::RenderText(std::string text, const glm::vec2& position, float scale, const glm::vec3& color)
{
   m_font.RenderText(GetProjection(), text, position, scale, color);
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
   glm::vec2 projectedPosition = GetProjection() * GetViewMatrix() * glm::vec4(globalPos, 0.0f, 1.0f);

   // convert to <0, 1>
   auto returnPos = (projectedPosition + glm::vec2(1.0f, 1.0f)) / 2.0f;

   // convert to <0, WIDTH>, <0, HEIGHT>
   // with y = 0 in top left corner
   const auto windowSize = GetWindowSize();
   returnPos.x *= windowSize.x;
   returnPos.y *= -windowSize.y;
   returnPos.y += windowSize.y;

   return returnPos;
}

glm::vec2
Application::ScreenToGlobal(const glm::vec2& screenPos)
{
   const auto windowCenterScreen = GetWindowSize() / 2.0f;
   const auto zoomRatio = (GetWindowSize() / (2.0f + GetZoomLevel())) / windowCenterScreen;

   // Compute distance from ceneter of screen to 'screenPos' value
   // Remember to multiply it by the current zoomRatio
   const auto distanceToObject = (screenPos - windowCenterScreen) * zoomRatio;

   // Rotate vector according to current camera's rotation
   const auto rotatedDistanceToObject = glm::rotateZ(glm::vec3(distanceToObject, 0.0f), m_camera.GetRotation());

   // Compute global position by adding computed distance to camera position (which is located in the center of the screen)
   const auto globalPos = m_camera.GetPosition() + rotatedDistanceToObject;

   return globalPos;
}

void
Application::DrawLine(glm::vec2 from, glm::vec2 to, glm::vec3 color)
{
   m_debugObjs.emplace_back(std::make_unique< Line >(from, to, color));
}

void
Application::PollEvents()
{
   glfwPollEvents();
}