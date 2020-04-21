#pragma once

#include "Camera.hpp"
#include "Font.hpp"
#include "InputManager.hpp"
#include "Level.hpp"
#include "Timer.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <memory>

class Player;
class Context;

#pragma region DEBUG
// THIS CLASS EXISTS ONLY IF WE WOULD NEED SOMETHING ELSE THAN LINE AS DEBUG OBJECT
class DebugObject
{
 public:
   virtual void
   Draw(Context& context) = 0;

   virtual void
   SetViewMatrix(const glm::mat4& mat) = 0;

   virtual ~DebugObject() = default;
};

class Context
{
 public:
   Context() = default;
   ~Context() = default;

   std::shared_ptr< Player >
   GetPlayer();

   Level&
   GetLevel();

   Camera&
   GetCamera();

   Timer::milliseconds
   GetDeltaTime();

   bool
   IsGame();

   void
   Log(Logger::TYPE t, const std::string& log);

   void
   RenderText(std::string text, const glm::vec2& position, float scale, const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f));

   // convert from global position (OpenGL) to screen position (in pixels)
   glm::vec2
   GlobalToScreen(const glm::vec2& globalPos) const;

   // convert from global position (OpenGL) to screen position (in pixels)
   glm::vec2
   ScreenToGlobal(const glm::vec2& screenPos);

   void
   CenterCameraOnPlayer();

   void
   DrawLine(glm::vec2 from, glm::vec2 to, glm::vec3 color = glm::vec3(1.0f, 0.0f, 0.0f), glm::mat4 view = glm::mat4(1.0f));

   virtual void
   MainLoop() = 0;

   virtual const glm::vec2&
   GetWindowSize() const = 0;

   virtual const glm::ivec2&
   GetFrameBufferwSize() const = 0;

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

   // DEBUG
   std::vector< std::unique_ptr< DebugObject > > m_debugObjs;

   Logger m_logger;
   float m_cameraSpeed = 0.0f;
   bool m_isGame = false;
   std::shared_ptr< Player > m_player = nullptr;
   Level m_currentLevel;
   Font m_font;

   InputManager m_inputManager;
   Camera m_camera;
   Timer m_timer;
   Timer::milliseconds m_deltaTime;
};


class Line : public DebugObject
{
   glm::vec2 m_from;
   glm::vec2 m_to;
   glm::vec3 m_color;

   glm::mat4 m_viewMatrix;
 public:
   Line(glm::vec2 from, glm::vec2 to, glm::vec3 color) : m_from(from), m_to(to), m_color(color)
   {
   }

   ~Line() override = default;

   void
   Draw(Context& context) override
   {
      Shaders lineShader{};
      lineShader.LoadShaders("lineShader");

      glm::vec2 vertices[2] = {m_from, m_to};

      // glm::mat4 modelMatrix = glm::scale(glm::mat4(), glm::vec3(1.0f, 1.0f, 1.0f));

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
      // lineShader.SetUniformFloatMat4(modelMatrix, "modelMatrix");
      lineShader.SetUniformFloatMat4(context.GetProjection(), "projectionMatrix");
      lineShader.SetUniformFloatMat4(m_viewMatrix, "viewMatrix");
      lineShader.SetUniformFloatVec4(glm::vec4(m_color, 1.0f), "color");

      glDrawArrays(GL_LINES, 0, 2);
      glBindVertexArray(0);
      glDeleteBuffers(1, &lineVertexBuffer);
      glDeleteVertexArrays(1, &lineVertexArray);
   }

   void
   SetViewMatrix(const glm::mat4& mat) override
   {
      m_viewMatrix = mat;
   }
};
#pragma endregion