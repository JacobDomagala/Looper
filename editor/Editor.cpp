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

   performLayout();
   setVisible(true);
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
Editor::HandleInput()
{
   m_timer.ToggleTimer();
   const auto cameraMovement = m_cameraSpeed * TARGET_TIME;

   auto cameraMoveBy = glm::ivec2();

   if (m_inputManager.CheckKeyPressed(GLFW_KEY_W))
   {
      cameraMoveBy += glm::ivec2(0, cameraMovement);
   }
   if (m_inputManager.CheckKeyPressed(GLFW_KEY_S))
   {
      cameraMoveBy += glm::ivec2(0, -cameraMovement);
   }
   if (m_inputManager.CheckKeyPressed(GLFW_KEY_A))
   {
      cameraMoveBy += glm::ivec2(cameraMovement, 0);
   }
   if (m_inputManager.CheckKeyPressed(GLFW_KEY_D))
   {
      cameraMoveBy += glm::ivec2(-cameraMovement, 0);
   }
   if (m_inputManager.CheckKeyPressed(GLFW_KEY_O))
   {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   }
   if (m_inputManager.CheckKeyPressed(GLFW_KEY_P))
   {
      glDisable(GL_BLEND);
      // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   }

   m_currentLevel.Move(cameraMoveBy);
   if (m_player)
   {
      m_player->Move(cameraMoveBy);
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
Editor::mouseButtonEvent(const nanogui::Vector2i& p, int button, bool down, int modifiers)
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
   }
   else
   {
      if (m_currentSelectedObject)
      {
         m_currentSelectedObject->SetColor({1.0f, 1.0f, 1.0f});
      }

      m_objectSelected = false;
   }

   return Screen::mouseButtonEvent(p, button, down, modifiers);
}

void
Editor::drawAll()
{
   // override keyboard input
   HandleInput();
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
   m_levelLoaded = true;
}

void
Editor::LoadLevel(const std::string& levelPath)
{
   m_levelFileName = levelPath;
   m_currentLevel.Load(*this, levelPath);
   m_player = m_currentLevel.GetPlayer();

   CenterCameraOnPlayer();

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
