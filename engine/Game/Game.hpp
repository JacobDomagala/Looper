#pragma once

#include "Font.hpp"
#include "Framebuffer.hpp"
#include "InputManager.hpp"
#include "Level.hpp"
#include "Logger.hpp"
#include "Player.hpp"
#include "Timer.hpp"
#include "Window.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

#pragma region DEBUG
// THIS CLASS EXISTS ONLY IF WE WOULD NEED SOMETHING ELSE THAN LINE AS DEBUG OBJECT
class DebugObject
{
 public:
   virtual void
   Draw(const glm::mat4& projection) = 0;
   virtual ~DebugObject() = default;
};

class Line : public DebugObject
{
   glm::vec2 m_from;
   glm::vec2 m_to;
   glm::vec3 m_color;

 public:
   Line(glm::vec2 from, glm::vec2 to, glm::vec3 color) : m_from(from), m_to(to), m_color(color)
   {
   }

   ~Line() override = default;

   void
   Draw(const glm::mat4& projection) override
   {
      Shaders lineShader{};
      lineShader.LoadShaders("lineVertex.glsl", "lineFragment.glsl");

      glm::vec2 vertices[2] = {m_from, m_to};

      glm::mat4 modelMatrix = glm::scale(glm::mat4(), glm::vec3(1.0f, 1.0f, 1.0f));

      GLuint lineVertexArray;
      GLuint lineVertexBuffer;

      glGenVertexArrays(1, &lineVertexArray);
      glGenBuffers(1, &lineVertexBuffer);
      glBindVertexArray(lineVertexArray);
      glBindBuffer(GL_ARRAY_BUFFER, lineVertexBuffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 2, vertices, GL_DYNAMIC_DRAW);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

      lineShader.UseProgram();
      lineShader.SetUniformFloatMat4(modelMatrix, "modelMatrix");
      lineShader.SetUniformFloatMat4(projection, "projectionMatrix");
      lineShader.SetUniformFloatVec4(glm::vec4(m_color, 1.0f), "color");

      glDrawArrays(GL_LINES, 0, 2);
      glBindVertexArray(0);
      glDeleteBuffers(1, &lineVertexBuffer);
      glDeleteVertexArrays(1, &lineVertexArray);
   }
};
#pragma endregion

class Game
{
 public:
   Game() = default;
   ~Game() = default;

   // Initialize Game using 'configFile'
   void
   Init(std::string configFile);

   Player&
   GetPlayer()
   {
      return *m_player;
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

   void
   SwapBuffers()
   {
      m_window->SwapBuffers();
   }

   bool
   IsRunning()
   {
      return m_window->IsRunning();
   }

   void
   Log(Logger::TYPE t, const std::string& log)
   {
      logger.Log(t, log);
   }

   void
   PollEvents()
   {
      glfwPollEvents();
   }

   void
   RegisterForKeyInput(IInputListener* listener)
   {
      m_inputManager.RegisterForKeyInput(listener);
   }

 private:
   // DEBUG
   std::vector< std::unique_ptr< DebugObject > > m_debugObjs;
   void
   RenderLine(const glm::ivec2& collided, const glm::vec3& color);

   enum class GameState : uint8_t
   {
      MENU = 0,
      GAME
   };

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
   LoadLevel(const std::string& levelName);

   void
   RayTracer();

   void
   HandleReverseLogic();

 private:
   Logger logger;
   std::unique_ptr< Window > m_window = nullptr;
   Timer m_timer;


   Level m_currentLevel;
   // all maps
   std::vector< std::string > m_levels;

   std::unique_ptr< byte_vec4 > m_collision;

   // active font used in game
   Font m_font;

   // how fast should camera move
   float m_cameraSpeed;

   // framebuffer for first pass
   Framebuffer m_frameBuffer;

   //
   float m_deltaTime;

   bool m_reverse = false;
   int m_frameCount = 0;

   // state of the game
   GameState m_state;

   // player position on map (centered)
   glm::vec2 m_playerPosition;
   std::unique_ptr< Player > m_player = nullptr;

   InputManager m_inputManager;
};
