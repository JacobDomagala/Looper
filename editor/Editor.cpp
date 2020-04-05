#include "Editor.hpp"
#include "Game.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <nanogui/button.h>
#include <nanogui/label.h>
#include <nanogui/layout.h>
#include <nanogui/window.h>
#include <nanovg.h>

#include <fstream>
#include <thread>

Editor::Editor() : nanogui::Screen(Eigen::Vector2i(1920, 1080), "DGame Editor", true, false, 8, 8, 24, 8, 0, 4, 5), m_gui(*this)
{
   InitGLFW();
   m_inputManager.Init(mGLFWWindow);

   m_projectionMatrix = glm::ortho(-1920.0f / 2.0f, 1920.0f / 2.0f, 1080.0f / 2.0f, -1080.0f / 2.0f, -1.0f, 1.0f);

   m_gui.Init();

   performLayout();
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
   const auto cameraMovement = 20;
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

   m_currentLevel.second.Move(cameraMoveBy);
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
      m_currentLevel.second.Render(m_projectionMatrix);
      //  draw(mNVGContext);
   }
}

void
Editor::LoadLevel(const std::string& levelPath)
{
   //const auto filename = std::filesystem::path(levelPath).filename().u8string();
   m_currentLevel.first = levelPath;
   m_currentLevel.second.Load(levelPath);
   m_levelLoaded = true;
}

void
Editor::PlayLevel()
{
   if (m_levelLoaded)
   {
      m_game.Init("GameInit.txt");
      m_game.LoadLevel(m_currentLevel.first);
      m_game.MainLoop();   
   }
}