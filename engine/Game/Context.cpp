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

glm::vec2
Context::GlobalToScreen(const glm::vec2& globalPos) const
{
   // convert to <-1, 1>
   glm::vec2 projectedPosition = GetProjection() * glm::vec4(globalPos, 0.0f, 1.0f);

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


void
Context::PollEvents()
{
   glfwPollEvents();
}