#include "Editor.hpp"
#include "Game.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <nanogui/button.h>
#include <nanogui/label.h>
#include <nanogui/layout.h>
#include <nanogui/window.h>
#include <nanovg.h>

#include <fstream>
#include <string>
#include <thread>

Editor::Editor(const glm::ivec2& screenSize)
   : nanogui::Screen(Eigen::Vector2i(screenSize.x, screenSize.y), "DGame Editor", true, false, 8, 8, 24, 8, 0, 4, 5), m_gui(*this)
{
   m_logger.Init("Editor");
   InitGLFW();
   m_inputManager.Init(mGLFWWindow);

   m_windowSize = screenSize;

   const auto left = -m_windowSize.x / 2.0f;
   const auto right = m_windowSize.x / 2.0f;
   const auto top = m_windowSize.y / 2.0f;
   const auto bottom = -m_windowSize.y / 2.0f;
   const auto near = -1.0f;
   const auto far = 1.0f;

   m_projectionMatrix = glm::ortho(left, right, top, bottom, near, far);

   m_gui.Init();
   m_font.SetFont("segoeui");
}

Editor::~Editor()
{
}

void
Editor::InitGLFW()
{
   if (GLFW_TRUE != glfwInit())
   {
      m_logger.Log(Logger::TYPE::FATAL, "GLFW_TRUE != glfwInit()");
   }

   int major, minor;
   glGetIntegerv(GL_MAJOR_VERSION, &major);
   glGetIntegerv(GL_MINOR_VERSION, &minor);
   m_logger.Log(Logger::TYPE::DEBUG, "OpenGL Version - " + std::to_string(major) + "." + std::to_string(minor));

   glewExperimental = GL_TRUE;

   if (glewInit() != GLEW_OK)
   {
      m_logger.Log(Logger::TYPE::FATAL, "glewInit() != GLEW_OK");
   }

   glEnable(GL_DEBUG_OUTPUT);
   glDebugMessageCallback(
      [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* logger) {
         std::string buffer(1024, 0x0);
         const auto newSize = sprintf(&buffer[0], "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s",
                                      (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);

         buffer.resize(newSize);
         reinterpret_cast< const Logger* >(logger)->Log(Logger::TYPE::DEBUG, buffer);
      },
      &m_logger);

   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_LINE_SMOOTH);
   glEnable(GL_MULTISAMPLE);
}

void
Editor::draw(NVGcontext* ctx)
{
   /* Draw the user interface */
   Screen::draw(ctx);
}

void
Editor::UpdateCamera()
{
   const auto viewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
   const auto upVector = glm::vec3(0.0f, -1.0f, 0.0f);
   const auto viewMatrix = glm::lookAt(m_cameraPosition, m_cameraPosition + viewDirection, upVector);
   m_currentLevel.GetShader().UseProgram();
   m_currentLevel.GetShader().SetUniformFloatMat4(viewMatrix, "viewMatrix");
}

void
Editor::HandleInput()
{
   m_timer.ToggleTimer();
   const auto cameraMovement = m_cameraSpeed * TARGET_TIME;

   auto cameraMoveBy = glm::vec2();

   if (m_inputManager.CheckKeyPressed(GLFW_KEY_W))
   {
      cameraMoveBy += glm::vec2(0, cameraMovement);
   }
   if (m_inputManager.CheckKeyPressed(GLFW_KEY_S))
   {
      cameraMoveBy += glm::vec2(0, -cameraMovement);
   }
   if (m_inputManager.CheckKeyPressed(GLFW_KEY_A))
   {
      cameraMoveBy += glm::vec2(cameraMovement, 0);
   }
   if (m_inputManager.CheckKeyPressed(GLFW_KEY_D))
   {
      cameraMoveBy += glm::vec2(-cameraMovement, 0);
   }
   if (m_inputManager.CheckKeyPressed(GLFW_KEY_ESCAPE))
   {
      if (m_objectSelected)
      {
         if (m_currentSelectedObject)
         {
            m_currentSelectedObject->SetColor({1.0f, 1.0f, 1.0f});
         }

         m_objectSelected = false;
         m_gui.GameObjectUnselected();
      }
   }


   if (m_levelLoaded)
   {
      //if (glm::length(cameraMoveBy) > 0)
      //{
      //   m_cameraPosition += glm::vec3(cameraMoveBy, 0.0f);
      //   UpdateCamera();
      //   Log(Logger::TYPE::INFO, std::to_string(cameraMoveBy.x) + " " + std::to_string(cameraMoveBy.y));
      //}
       m_currentLevel.Move(cameraMoveBy);

      if (m_player)
      {
         m_player->Move(cameraMoveBy);
      }
   }

}

bool
Editor::scrollEvent(const nanogui::Vector2i& p, const nanogui::Vector2f& rel)
{
   m_zoomScale += rel.y() * 0.25f;

   m_zoomScale = std::clamp(m_zoomScale, m_maxZoomOut, m_maxZoomIn);

   const auto left = -m_windowSize.x / (2.0f + m_zoomScale);
   const auto right = m_windowSize.x / (2.0f + m_zoomScale);
   const auto top = m_windowSize.y / (2.0f + m_zoomScale);
   const auto bottom = -m_windowSize.y / (2.0f + m_zoomScale);
   const auto near = -1.0f;
   const auto far = 1.0f;

   m_projectionMatrix = glm::ortho(left, right, top, bottom, near, far);

   return Screen::scrollEvent(p, rel);
}

bool
Editor::mouseMotionEvent(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers)
{
   const auto currentCursorPosition = glm::vec2(p.x(), p.y());

   if (m_mousePressedLastUpdate && m_levelLoaded)
   {
      auto mouseMovementLength = glm::length(currentCursorPosition - m_lastCursorPosition);
      mouseMovementLength = glm::clamp(mouseMovementLength, 0.0f, 100.0f);

      m_mouseDrag = true;

      if (m_inputManager.CheckKeyPressed(GLFW_KEY_LEFT_CONTROL))
      {
         m_logger.Log(Logger::TYPE::INFO, "Sprite rotation");
         /*const auto moveVector = glm::vec2(rel.x(), rel.y()) * mouseMovementLength;
         const auto tmpVal = GetProjection() * glm::vec4(moveVector, 0.0f, 1.0f);
         const auto angle = glm::degrees(glm::atan(tmpVal.x, tmpVal.y));
         m_currentLevel.Rotate(0.4f, true);*/
      }
      else
      {
         m_logger.Log(Logger::TYPE::INFO, "Camera movement");
         m_currentLevel.Move(glm::vec2(rel.x(), rel.y()) * mouseMovementLength);

         if (m_player)
         {
            m_player->Move(glm::vec2(rel.x(), rel.y()) * mouseMovementLength);
         }
      }
   }

   m_lastCursorPosition = currentCursorPosition;
   return Screen::mouseMotionEvent(p, rel, button, modifiers);
}

bool
Editor::mouseButtonEvent(const nanogui::Vector2i& p, int button, bool down, int modifiers)
{
   Log(Logger::TYPE::INFO, "Mouse click event");

   m_mousePressedLastUpdate = down;

   if (down)
   {
      auto newSelectedObject = m_currentLevel.GetGameObjectOnLocation(glm::vec2(p.x(), p.y()));

      if (newSelectedObject)
      {
         if (m_currentSelectedObject != newSelectedObject)
         {
            if (m_currentSelectedObject)
            {
               m_currentSelectedObject->SetColor({1.0f, 1.0f, 1.0f});
            }

            m_currentSelectedObject = newSelectedObject;
         }

         m_currentSelectedObject->SetColor({1.0f, 0.0f, 0.0f});

         m_objectSelected = true;

         m_gui.GameObjectSelected(m_currentSelectedObject);
      }
   }


   return Screen::mouseButtonEvent(p, button, down, modifiers);
}

bool
Editor::mouseDragEvent(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers)
{
   Log(Logger::TYPE::INFO,
       std::to_string(p.x()) + "/" + std::to_string(p.x()) + "  " + std::to_string(rel.x()) + "/" + std::to_string(rel.x()));
   return Screen::mouseDragEvent(p, rel, button, modifiers);
}

void
Editor::drawAll()
{
   // override keyboard input
   HandleInput();

   // Update UI
   performLayout();

   Screen::drawAll();
}

void
Editor::drawContents()
{
   if (m_levelLoaded)
   {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      m_currentLevel.Render(m_projectionMatrix);
   }
}

void
Editor::CreateLevel(const glm::ivec2& size)
{
   m_currentLevel.Create(size);

   m_currentLevel.GetShader().SetUniformBool(false, "outlineActive");
   m_levelLoaded = true;
}

void
Editor::LoadLevel(const std::string& levelPath)
{
   m_levelFileName = levelPath;
   m_currentLevel.Load(*this, levelPath);
   m_player = m_currentLevel.GetPlayer();

   CenterCameraOnPlayer();

   m_currentLevel.GetShader().SetUniformBool(false, "outlineActive");

   //const auto position = glm::vec3(0.0f, 0.0f, 0.0f);
   //const auto viewDirection = glm::vec3(0.0f, 0.0f, 1.0f);
   //const auto upVector = glm::vec3(0.0f, 1.0f, 0.0f);
   //const auto viewMatrix = glm::lookAt(position, position + viewDirection, upVector);
   //m_currentLevel.GetShader().SetUniformFloatMat4(viewMatrix, "viewMatrix");

   m_levelLoaded = true;
}

void
Editor::SaveLevel(const std::string& levelPath)
{
   m_currentLevel.Save(levelPath);
}

void
Editor::PlayLevel()
{
   if (m_levelLoaded)
   {
      m_game.Init("GameInit.txt");
      m_game.LoadLevel(m_levelFileName);
      m_game.MainLoop();
   }
}

void
Editor::ShowWireframe(int wireframeEnabled)
{
   m_currentLevel.GetShader().UseProgram();
   m_currentLevel.GetShader().SetUniformBool(wireframeEnabled, "outlineActive");
}

const glm::vec2&
Editor::GetWindowSize() const
{
   return m_windowSize;
}

const glm::mat4&
Editor::GetProjection() const
{
   return m_projectionMatrix;
}

float
Editor::GetZoomLevel()
{
   return m_zoomScale;
}

bool
Editor::IsRunning()
{
   return m_isRunning;
}

void
Editor::MainLoop()
{
   Logger::SetLogType(Logger::TYPE::INFO);

   while (IsRunning())
   {
      PollEvents();
      drawAll();
   }
}
