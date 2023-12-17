#pragma once

#include "input_listener.hpp"
#include "level.hpp"
#include "logger.hpp"
#include "renderer/camera/camera.hpp"
#include "utils/time/timer.hpp"
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

   [[nodiscard]] const Player&
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
   GetFramesLastSecond() const;

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
   WindowFocusCallback(WindowFocusEvent& event) override;

 protected:
   [[nodiscard]] virtual bool
   IsRunning() const = 0;

   bool isGame_ = false;
   bool windowInFocus_ = true;

   std::shared_ptr< Level > currentLevel_ = nullptr;

   std::unique_ptr< renderer::Window > window_ = {};
   renderer::Camera camera_ = {};
   time::Timer timer_ = {};
   time::milliseconds deltaTime_ = {};
   int32_t frames_ = 0;
   float frameTimer_ = 0.0f;
   int32_t framesLastSecond_ = 0;
   uint32_t numObjects_ = {};

   WorkQueue workQueue_ = {};
};


} // namespace looper
