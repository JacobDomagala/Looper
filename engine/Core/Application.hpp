#pragma once

#include "Camera.hpp"
// #include "Font.hpp"
#include "InputListener.hpp"
#include "Level.hpp"
#include "logger.hpp"
#include "timer.hpp"

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
   Application&
   operator=(Application&) = delete;
   Application&
   operator=(Application&&) = delete;
   Application(Application&) = delete;
   Application(Application&&) = delete;


   [[nodiscard]] std::shared_ptr< Player >
   GetPlayer();

   [[nodiscard]] Window&
   GetWindow();

   [[nodiscard]] Level&
   GetLevel();

   [[nodiscard]] Camera&
   GetCamera();

   [[nodiscard]] Timer::milliseconds
   GetDeltaTime() const;

   [[nodiscard]] bool
   IsGame() const;

   virtual void
   Render(VkCommandBuffer cmdBuffer) = 0;

   // convert from global position (OpenGL) to screen position (in pixels)
   [[nodiscard]] glm::vec2
   GlobalToScreen(const glm::vec2& globalPos) const;

   // convert from screen position (in pixels) to global position (OpenGL)
   [[nodiscard]] glm::vec2
   ScreenToGlobal(const glm::vec2& screenPos) const;

   void
   CenterCameraOnPlayer();

   virtual void
   MainLoop() = 0;

   [[nodiscard]] virtual glm::vec2
   GetWindowSize() const = 0;

   [[nodiscard]] virtual const glm::mat4&
   GetProjection() const = 0;

   [[nodiscard]] virtual const glm::mat4&
   GetViewMatrix() const = 0;

   [[nodiscard]] virtual float
   GetZoomLevel() const = 0;

 protected:
   [[nodiscard]] virtual bool
   IsRunning() const = 0;

   bool m_isGame = false;
   std::shared_ptr< Player > m_player = nullptr;
   std::shared_ptr< Level > m_currentLevel = nullptr;

   std::unique_ptr< Window > m_window = nullptr;
   Camera m_camera = {};
   Timer m_timer = {};
   Timer::milliseconds m_deltaTime = {};
};


} // namespace looper
