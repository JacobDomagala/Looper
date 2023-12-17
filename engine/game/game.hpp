#pragma once

#include "application.hpp"
#include "input_manager.hpp"
#include "level.hpp"
#include "logger.hpp"
#include "player.hpp"
#include "utils/time/timer.hpp"
#include "renderer/window/window.hpp"
#include "state_list.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

namespace looper {

class Game : public Application
{
 public:
   void
   MainLoop() override;

   // Initialize Game using 'configFile'
   void
   Init(const std::string& configFile, bool loadLevel = true);

   [[nodiscard]] bool
   IsReverse() const;

   void
   ProcessInput(time::milliseconds deltaTime);

   void
   KeyCallback(KeyEvent& event) override;

   void
   Render(VkCommandBuffer cmdBuffer) override;

   [[nodiscard]] glm::vec2
   GetCursor();

   [[nodiscard]] glm::vec2
   GetCursorScreenPosition();

   void
   LoadLevel(const std::string& pathToLevel);

   [[nodiscard]] glm::vec2
   GetWindowSize() const override;

   [[nodiscard]] const glm::mat4&
   GetProjection() const override;

   [[nodiscard]] const glm::mat4&
   GetViewMatrix() const override;

   [[nodiscard]] float
   GetZoomLevel() const override;

 private:
   enum class GameState : uint8_t
   {
      MENU = 0,
      GAME
   };

   /**
    * \brief Check bullet collision along the line, starting from \c position
    * with length of \c range
    *
    * \return Returns collided position (or \c position + \c range if not collided)
    */
   //glm::vec2
   //CheckBulletCollision(const glm::vec2& positon, float range);

   /**
    * \brief Try to move GameObject \c gameObject by \c moveBy vector
    * \param[in] gameObject Object to move
    * \param[in] moveBy Movement vector
    */
   void
   MoveGameObject(Object::ID gameObject, const glm::vec2& moveBy) const;

   void
   KeyEvents();

   void
   MouseEvents();

   // Updates the internal states of each game object
   // Needed for reversing time
   void
   UpdateGameState();

   // draws to framebuffer (texture)
   void
   RenderFirstPass();

   // draws to screen
   void
   RenderSecondPass();

   void
   HandleReverseLogic();

   [[nodiscard]] bool
   IsRunning() const override;

   bool m_initialized = false;

   // all maps
   std::vector< std::string > m_levels = {};

   // set to true when game runs in reverse mode
   bool m_reverse = false;

   // number of frames saved for reverse mode (max value NUM_FRAMES_TO_SAVE)
   int m_frameCount = 0;

   // state of the game
   GameState m_state = GameState::GAME;

   StateList< glm::vec2 > cameraPositions_ = {};
};

} // namespace looper
