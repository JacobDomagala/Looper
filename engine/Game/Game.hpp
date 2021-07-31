#pragma once

#include "Application.hpp"
#include "Font.hpp"
#include "Framebuffer.hpp"
#include "InputManager.hpp"
#include "Level.hpp"
#include "Logger.hpp"
#include "Player.hpp"
#include "Timer.hpp"
#include "Window.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

namespace dgame {

class Game : public Application
{
 public:

   void
   MainLoop() override;

   // Initialize Game using 'configFile'
   void
   Init(std::string configFile);

   bool
   IsReverse() const;

   void
   ProcessInput(Timer::milliseconds deltaTime);

   void
   Render();

   glm::vec2
   GetCursor();

   glm::vec2
   GetCursorScreenPosition();

   void
   SwapBuffers();

   void
   RegisterForKeyInput(InputListener* listener);

   void
   LoadLevel(const std::string& pathToLevel);

   glm::vec2
   GetWindowSize() const override;

   const glm::mat4&
   GetProjection() const override;

   const glm::mat4&
   GetViewMatrix() const override;

   float
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
   glm::vec2
   CheckBulletCollision(const glm::vec2& positon, float range);

   /**
    * \brief Try to move GameObject \c gameObject by \c moveBy vector
    * \param[in] gameObject Object to move
    * \param[in] moveBy Movement vector
    */
   void
   MoveGameObject(GameObject* gameObject, const glm::vec2& moveBy) const;

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

   bool
   IsRunning() const override;

 private:
   bool m_initialized = false;

   int32_t m_frames = 0;
   float m_frameTimer = 0.0f;
   int32_t m_framesLastSecond = 0;

   // all maps
   std::vector< std::string > m_levels;

   // framebuffer for first pass
   Framebuffer m_frameBuffer;

   // set to true when game runs in reverse mode
   bool m_reverse = false;

   // number of frames saved for reverse mode (max value NUM_FRAMES_TO_SAVE)
   int m_frameCount = 0;

   // state of the game
   GameState m_state = GameState::GAME;
};

} // namespace dgame