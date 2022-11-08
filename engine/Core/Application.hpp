#pragma once

#include "Camera.hpp"
#include "Font.hpp"
#include "InputListener.hpp"
#include "Level.hpp"
#include "Logger.hpp"
#include "Timer.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <memory>

namespace looper {

class Player;
class Application;

class Application : public InputListener
{
 public:
   Application() = default;
   ~Application() override;
   Application&
   operator=(Application&) = delete;
   Application&
   operator=(Application&&) = delete;
   Application(Application&) = delete;
   Application(Application&&) = delete;


   std::shared_ptr< Player >
   GetPlayer();

   Window&
   GetWindow();

   Level&
   GetLevel();

   Camera&
   GetCamera();

   Timer::milliseconds
   GetDeltaTime() const;

   bool
   IsGame() const;

   /*void
   Log(Logger::Type t, const std::string& log) const;*/

   void
   RenderText(const std::string& text, const glm::vec2& position, float scale,
              const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f));

   // convert from global position (OpenGL) to screen position (in pixels)
   glm::vec2
   GlobalToScreen(const glm::vec2& globalPos) const;

   // convert from screen position (in pixels) to global position (OpenGL)
   glm::vec2
   ScreenToGlobal(const glm::vec2& screenPos) const;

   void
   CenterCameraOnPlayer();

   virtual void
   MainLoop() = 0;

   virtual glm::vec2
   GetWindowSize() const = 0;

   virtual const glm::mat4&
   GetProjection() const = 0;

   virtual const glm::mat4&
   GetViewMatrix() const = 0;

   virtual float
   GetZoomLevel() const = 0;

 protected:
   virtual bool
   IsRunning() const = 0;

   // NOLINTBEGIN (cppcoreguidelines-non-private-member-variables-in-classes)
   Logger m_logger;
   bool m_isGame = false;
   std::shared_ptr< Player > m_player = nullptr;
   std::shared_ptr< Level > m_currentLevel;
   Font m_font;

   std::unique_ptr< Window > m_window;
   Camera m_camera;
   Timer m_timer;
   Timer::milliseconds m_deltaTime = {};
   // NOLINTEND
};


} // namespace looper