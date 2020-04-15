#include "Editor.hpp"
#include "Game.hpp"

#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <nanogui/button.h>
#include <nanogui/label.h>
#include <nanogui/layout.h>
#include <nanogui/window.h>
#include <nanovg.h>
#include <string>
#include <thread>

Editor::Editor(const glm::vec2& screenSize)
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
   const auto far = 10.0f;

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
Editor::HandleInput()
{
   m_timer.ToggleTimer();

   auto cameraMoveBy = glm::vec2();

   if (m_inputManager.CheckKeyPressed(GLFW_KEY_W))
   {
      cameraMoveBy += glm::vec2(0.0f, -1.0f);
   }
   if (m_inputManager.CheckKeyPressed(GLFW_KEY_S))
   {
      cameraMoveBy += glm::vec2(0.0f, 1.0f);
   }
   if (m_inputManager.CheckKeyPressed(GLFW_KEY_A))
   {
      cameraMoveBy += glm::vec2(-1.0f, 0.0f);
   }
   if (m_inputManager.CheckKeyPressed(GLFW_KEY_D))
   {
      cameraMoveBy += glm::vec2(1.0f, 0);
   }
   if (m_inputManager.CheckKeyPressed(GLFW_KEY_SPACE))
   {
      if (m_levelLoaded)
      {
         m_camera.SetCameraAtPosition({0.0f, 0.0f, 0.0f});
      }
   }
   if (m_inputManager.CheckKeyPressed(GLFW_KEY_ESCAPE))
   {
      if (m_objectSelected)
      {
         if (m_currentSelectedObject)
         {
            m_currentSelectedObject->SetObjectUnselected();
         }

         m_objectSelected = false;
         m_gui.GameObjectUnselected();
      }
   }

   if (m_levelLoaded)
   {
      m_camera.Move(glm::vec3(cameraMoveBy, 0.0f));

      glm::dvec2 cursorPos;
      glfwGetCursorPos(mGLFWWindow, &cursorPos.x, &cursorPos.y);

      m_currentLevel.GetGameObjectOnLocation(cursorPos);
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

void
Editor::ShowCursor(bool choice)
{
   int mode = choice ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN;
   glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, mode);
}

void
Editor::HandleMouseDrag(const glm::vec2& currentCursorPos, const glm::vec2& axis)
{
   if (!m_movementOnObject)
   {
      m_movementOnObject = m_objectSelected && (m_currentSelectedObject == m_currentLevel.GetGameObjectOnLocation(currentCursorPos));
   }

   // Rotate camera (or currently selected Object)
   if (m_inputManager.CheckKeyPressed(GLFW_KEY_LEFT_CONTROL))
   {
      // Calculate the value of cursor movement
      // For example:
      // - cursor was moved to the right then movementVector.x is positive, negative otherwise
      // - cursor was moved to the top of window then movementVector.y is positive, negative otherwise
      const auto movementVector = currentCursorPos - m_lastCursorPosition;

      const auto maxRotationAngle = 0.025f;
      const auto angle = glm::clamp(axis.x ? movementVector.x : -movementVector.y, -maxRotationAngle, maxRotationAngle);

      m_movementOnObject ? m_currentSelectedObject->Rotate(-angle, true) : m_camera.Rotate(angle);
   }
   // Move camera (or currently selected Object)
   else
   {
      auto mouseMovementLength = glm::length(currentCursorPos - m_lastCursorPosition);

      const auto minCameraMovement = 1.0f;
      const auto maxCameraMovement = 2.0f;

      mouseMovementLength = glm::clamp(mouseMovementLength, minCameraMovement, maxCameraMovement);

      const auto moveBy = glm::vec3(-axis.x, -axis.y, 0.0f);

      m_movementOnObject ? m_currentSelectedObject->Move(m_camera.ConvertToCameraVector(-moveBy)) : m_camera.Move(moveBy);
   }

   m_mouseDrag = true;
}

bool
Editor::mouseMotionEvent(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers)
{
   const auto currentCursorPosition = glm::vec2(p.x(), p.y());

   if (m_mousePressedLastUpdate && m_levelLoaded)
   {
      ShowCursor(false);
      HandleMouseDrag(currentCursorPosition, {rel.x(), rel.y()});
   }
   else
   {
      ShowCursor(true);
   }

   m_lastCursorPosition = currentCursorPosition;
   return Screen::mouseMotionEvent(p, rel, button, modifiers);
}

void
Editor::HandleObjectSelected(std::shared_ptr< GameObject > newSelectedObject)
{
   if (m_currentSelectedObject != newSelectedObject)
   {
      if (m_currentSelectedObject)
      {
         // unselect previously selected object
         m_currentSelectedObject->SetObjectUnselected();
      }

      m_currentSelectedObject = newSelectedObject;
   }

   // mark new object as selected
   m_currentSelectedObject->SetObjectSelected();
   m_gui.GameObjectSelected(m_currentSelectedObject);
   m_objectSelected = true;
}

void
Editor::CheckIfObjectGotSelected(const glm::vec2& cursorPosition)
{
   auto newSelectedObject = m_currentLevel.GetGameObjectOnLocation(cursorPosition);

   if (newSelectedObject)
   {
      HandleObjectSelected(newSelectedObject);
   }
}

bool
Editor::mouseButtonEvent(const nanogui::Vector2i& position, int button, bool down, int modifiers)
{
   m_mousePressedLastUpdate = down;

   if (down)
   {
      CheckIfObjectGotSelected({position.x(), position.y()});
   }
   else
   {
      m_movementOnObject = false;
      ShowCursor(true);
   }

   return Screen::mouseButtonEvent(position, button, down, modifiers);
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
      m_currentLevel.Render();

      for (auto& obj : m_debugObjs)
      {
         obj->Draw(*this);
      }
      m_debugObjs.clear();
   }
}

void
Editor::CreateLevel(const glm::ivec2& size)
{
   if (m_levelLoaded)
   {
      m_currentLevel.Quit();
   }

   m_currentLevel.Create(this, size);
   m_currentLevel.LoadShaders("Editor");


   m_camera.Create(glm::vec3(m_currentLevel.GetLevelPosition(), 0.0f), {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f});

   m_levelLoaded = true;
}

void
Editor::LoadLevel(const std::string& levelPath)
{
   if (m_levelLoaded)
   {
      m_currentLevel.Quit();
   }

   m_levelFileName = levelPath;
   m_currentLevel.Load(this, levelPath);
   m_currentLevel.LoadShaders("Editor");

   m_camera.Create(glm::vec3(m_currentLevel.GetPlayer()->GetCenteredGlobalPosition(), 0.0f), {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f});

   m_levelLoaded = true;
}

void
Editor::SaveLevel(const std::string& levelPath)
{
   m_currentLevel.Save(levelPath);
}

void
Editor::AddGameObject(GameObject::TYPE objectType)
{
   HandleObjectSelected(m_currentLevel.AddGameObject(objectType));
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

const glm::mat4&
Editor::GetViewMatrix() const
{
   return m_camera.GetViewMatrix();
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
