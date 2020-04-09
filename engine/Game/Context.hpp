#pragma once

#include "Font.hpp"
#include "InputManager.hpp"
#include "Level.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <memory>

class Player;

class Context
{
 public:
   Context() = default;
   ~Context() = default;

   std::shared_ptr< Player >
   GetPlayer();

   Level&
   GetLevel();

   bool
   IsGame();

   void
   Log(Logger::TYPE t, const std::string& log);

   void
   RenderText(std::string text, const glm::vec2& position, float scale, const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f));

   virtual const glm::mat4&
   GetProjection() const = 0;

 protected:
   Logger m_logger;
   float m_cameraSpeed = 0.0f;
   bool m_isGame = false;
   std::shared_ptr< Player > m_player = nullptr;
   Level m_currentLevel;
   Font m_font;

   InputManager m_inputManager;
};
