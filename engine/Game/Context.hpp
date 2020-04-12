#pragma once

#include "Font.hpp"
#include "InputManager.hpp"
#include "Level.hpp"
#include "Camera.hpp"
#include "Timer.hpp"

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

   // convert from global position (OpenGL) to screen position (in pixels)
   glm::vec2
   GlobalToScreen(const glm::vec2& globalPos) const;

   void
   CenterCameraOnPlayer();

   virtual void
   MainLoop() = 0;

   virtual const glm::vec2&
   GetWindowSize() const = 0;

   virtual const glm::mat4&
   GetProjection() const = 0;

   virtual const glm::mat4&
   GetViewMatrix() const = 0;

   virtual float
   GetZoomLevel() = 0;

 protected:
   virtual bool
   IsRunning() = 0;

   void
   PollEvents();

   Logger m_logger;
   float m_cameraSpeed = 0.0f;
   bool m_isGame = false;
   std::shared_ptr< Player > m_player = nullptr;
   Level m_currentLevel;
   Font m_font;

   InputManager m_inputManager;
   Camera m_camera;
   Timer m_timer;
};
