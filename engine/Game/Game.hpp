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
   Game() = default;
   ~Game() override = default;

   void
   MainLoop() override;

   // Initialize Game using 'configFile'
   void
   Init(std::string configFile);

   bool
   IsReverse() const;

   std::pair< glm::ivec2, bool >
   CheckBulletCollision(Enemy* from, glm::vec2 targetPosition, int32_t range);

   bool
   IsPlayerInVision(Enemy* from, int32_t range);

   glm::ivec2
   CheckCollision(const glm::ivec2& currentPosition, const glm::ivec2& moveBy);

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
   LoadLevel(const std::string& levelName);

   // TODO move all collision related code to Level class?
   void
   SetCollisionMap(byte_vec4* collision);

   glm::vec2
   GetWindowSize() const override;

   const glm::mat4&
   GetProjection() const override;

   const glm::mat4&
   GetViewMatrix() const override;

   float
   GetZoomLevel() override;

 private:
   enum class GameState : uint8_t
   {
      MENU = 0,
      GAME
   };

   void
   RenderLine(const glm::ivec2& collided, const glm::vec3& color);

   // bullet collision for player
   glm::ivec2
   CheckBulletCollision(int32_t range);

   glm::ivec2
   CheckCollision(glm::ivec2& moveBy);

   glm::ivec2
   CorrectPosition();

   bool
   CheckMove(glm::vec2& moveBy);

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
   IsRunning() override;

 private:
   bool m_initialized = false;

   int32_t m_frames = 0;
   float m_frameTimer = 0.0f;
   int32_t m_framesLastSecond = 0;

   // all maps
   std::vector< std::string > m_levels;

   // TODO: Move entire collision logic to Level class
   byte_vec4* m_collision = nullptr;

   // framebuffer for first pass
   Framebuffer m_frameBuffer;

   // set to true when game runs in reverse mode
   bool m_reverse = false;

   // number of frames saved for reverse mode (max value NUM_FRAMES_TO_SAVE)
   int m_frameCount = 0;

   // state of the game
   GameState m_state = GameState::GAME;

   // player position on map (centered)
   glm::vec2 m_playerPosition;
};

} // namespace dgame