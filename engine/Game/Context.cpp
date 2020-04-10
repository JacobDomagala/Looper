#include "Context.hpp"

#include <GLFW/glfw3.h>

std::shared_ptr< Player >
Context::GetPlayer()
{
   return m_player;
}

Level&
Context::GetLevel()
{
   return m_currentLevel;
}

bool
Context::IsGame()
{
   return m_isGame;
}

void
Context::Log(Logger::TYPE t, const std::string& log)
{
   m_logger.Log(t, log);
}

void
Context::RenderText(std::string text, const glm::vec2& position, float scale, const glm::vec3& color)
{
   m_font.RenderText(GetProjection(), text, position, scale, color);
}

void
Context::CenterCameraOnPlayer()
{
   m_currentLevel.Move(-m_player->GetCenteredGlobalPosition());
   m_player->Move(-m_player->GetCenteredGlobalPosition());
}

void
Context::PollEvents()
{
   glfwPollEvents();
}