#pragma once

#include <Font.hpp>
#include <Framebuffer.hpp>
#include <Level.hpp>
#include <Player.hpp>
#include <Timer.hpp>
#include <Window.hpp>
#include <glm/glm.hpp>
#include <vector>

class DebugObject;

class Game
{
   // DEBUG
   std::vector< std::unique_ptr< DebugObject > > m_debugObjs;
   void
   RenderLine(const glm::ivec2& collided, const glm::vec3& color);

   enum class GameState : uint8_t
   {
      MENU = 0,
      GAME,
      EDITOR
   };

   Timer m_timer;
   Level m_currentLevel;

   std::unique_ptr< byte_vec4 > m_collision;

   // active font used in game
   Font m_font;

   // how fast should camera move
   float m_cameraSpeed;

   // framebuffer for first pass
   Framebuffer m_frameBuffer;

   //
   float m_deltaTime;

   // all maps
   std::vector< std::string > m_levels;

   // state of the game
   GameState m_state;
   Player m_player;

   // player position on map (centered)
   glm::vec2 m_playerPosition;

   Window* m_window = nullptr;

   // bullet collision for player
   glm::ivec2
   CheckBulletCollision(int32_t range);
   glm::ivec2
   CheckCollision(glm::ivec2& moveBy);
   glm::ivec2
   CorrectPosition();

   // convert from global position (OpenGL) to screen position (in pixels)
   glm::ivec2
   GlobalToScreen(glm::vec2 globalPos);

   bool
   CheckMove(glm::ivec2& moveBy);
   void
   KeyEvents(float deltaTime);
   void
   MouseEvents(float deltaTime);

   // draws to framebuffer (texture)
   void
   RenderFirstPass();

   // draws to screen
   void
   RenderSecondPass();
   void
   LoadLevel(const std::string& levelName);

   void
   RayTracer();

   Game();

 public:
   // Singleton for Game class
   static Game&
   GetInstance();

   // Initialize Game using 'configFile'
   void
   Init(std::string configFile, Window* windowPtr);

   ~Game() = default;

   // Deleted copy constructor
   Game(Game&) = delete;

   Player&
   GetPlayer()
   {
      return m_player;
   }

   Level&
   GetLevel()
   {
      return m_currentLevel;
   }

   float
   GetDeltaTime() const
   {
      return m_deltaTime;
   }

   std::pair< glm::ivec2, bool >
   CheckBulletCollision(Enemy* from, glm::vec2 targetPosition, int32_t range);
   bool
   IsPlayerInVision(Enemy* from, int32_t range);
   glm::ivec2
   CheckCollision(const glm::ivec2& currentPosition, const glm::ivec2& moveBy);

   void
   DrawLine(glm::vec2 from, glm::vec2 to, glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f));
   void
   RenderText(std::string text, const glm::vec2& position, float scale, const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f));

   void
   ProcessInput(float deltaTime);
   void
   Render();

   const glm::mat4&
   GetProjection() const
   {
      return m_window->GetProjection();
   }

   glm::vec2
   GetCursor()
   {
      return m_window->GetCursor();
   }

   glm::vec2
   GetCursorScreenPosition()
   {
      return m_window->GetCursorScreenPosition();
   }

};
