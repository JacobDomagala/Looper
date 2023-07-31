#pragma once

#include "input_listener.hpp"
#include "level.hpp"
#include "utils/time/timer.hpp"
#include "logger.hpp"
#include "renderer/camera/camera.hpp"
#include "work_queue.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <memory>

namespace looper::renderer {
class Window;
}

namespace looper {

class Player;
class Application;

class Application : public InputListener
{
 public:
   Application() = default;
   ~Application() override = default;

   Application&
   operator=(Application&) = delete;
   Application&
   operator=(Application&&) = delete;
   Application(Application&) = delete;
   Application(Application&&) = delete;

   [[nodiscard]] std::shared_ptr< Player >
   GetPlayer();

   [[nodiscard]] renderer::Window&
   GetWindow();

   [[nodiscard]] Level&
   GetLevel();

   [[nodiscard]] renderer::Camera&
   GetCamera();

   [[nodiscard]] time::milliseconds
   GetDeltaTime() const;

   [[nodiscard]] int32_t
   GetFramesLastSecond() const
   {
      return m_framesLastSecond;
   }

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

   void
   WindowFocusCallback(const WindowFocusEvent& event) override;

 protected:
   [[nodiscard]] virtual bool
   IsRunning() const = 0;

   bool m_isGame = false;
   bool windowInFocus_ = true;

   std::shared_ptr< Player > m_player = nullptr;
   std::shared_ptr< Level > m_currentLevel = nullptr;

   std::unique_ptr< renderer::Window > m_window = {};
   renderer::Camera m_camera = {};
   time::Timer m_timer = {};
   time::milliseconds m_deltaTime = {};
   int32_t m_frames = 0;
   float m_frameTimer = 0.0f;
   int32_t m_framesLastSecond = 0;
   uint32_t numObjects_ = {};

   WorkQueue workQueue_ = {};
};


} // namespace looper
