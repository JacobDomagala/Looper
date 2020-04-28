#include "Editor.hpp"
#include "Enemy.hpp"
#include "Game.hpp"

#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <nanogui/button.h>
#include <nanogui/label.h>
#include <nanogui/layout.h>
#include <nanogui/window.h>
#include <nanovg.h>
#include <string>

Editor::Editor(const glm::vec2& screenSize)
   : nanogui::Screen(Eigen::Vector2i(screenSize.x, screenSize.y), "DGame Editor", true, false, 8, 8, 24, 8, 0, 4, 5), m_gui(*this)
{
   m_logger.Init("Editor");
   InitGLFW();
   // m_inputManager.Init(mGLFWWindow);

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

   m_deltaTime = Timer::milliseconds(static_cast< long >(TARGET_TIME * 1000));
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
Editor::ShowCursor(bool choice)
{
   int mode = choice ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
   glfwSetInputMode(mGLFWWindow, GLFW_CURSOR, mode);
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
   m_deltaTime = m_timer.GetMsDeltaTime();

   auto cameraMoveBy = glm::vec2();

   if (IsKeyDown(GLFW_KEY_W))
   {
      cameraMoveBy += glm::vec2(0.0f, -1.0f);
   }
   if (IsKeyDown(GLFW_KEY_S))
   {
      cameraMoveBy += glm::vec2(0.0f, 1.0f);
   }
   if (IsKeyDown(GLFW_KEY_A))
   {
      cameraMoveBy += glm::vec2(-1.0f, 0.0f);
   }
   if (IsKeyDown(GLFW_KEY_D))
   {
      cameraMoveBy += glm::vec2(1.0f, 0);
   }
   if (IsKeyDown(GLFW_KEY_SPACE))
   {
      if (m_levelLoaded)
      {
         m_camera.SetCameraAtPosition({0.0f, 0.0f, 0.0f});
      }
   }
   if (IsKeyDown(GLFW_KEY_ESCAPE))
   {
      if (m_gameObjectSelected)
      {
         if (m_currentSelectedGameObject)
         {
            m_currentSelectedGameObject->SetObjectUnselected();
            m_currentSelectedGameObject.reset();
         }

         m_gameObjectSelected = false;
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
   if (!Screen::scrollEvent(p, rel))
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
   }

   return true;
}

void
Editor::HandleMouseDrag(const glm::vec2& currentCursorPos, const glm::vec2& axis)
{
   // Rotate camera (or currently selected Object)
   if (IsKeyDown(GLFW_KEY_LEFT_CONTROL))
   {
      // Calculate the value of cursor movement
      // For example:
      // - cursor was moved to the right then movementVector.x is positive, negative otherwise
      // - cursor was moved to the top of window then movementVector.y is positive, negative otherwise
      const auto movementVector = currentCursorPos - m_lastCursorPosition;

      const auto maxRotationAngle = 0.025f;
      const auto angle = glm::clamp(axis.x ? movementVector.x : -movementVector.y, -maxRotationAngle, maxRotationAngle);

      if (m_movementOnEditorObject || m_movementOnGameObject)
      {
         // Editor objects selected have higher priority of movement
         // for example when animation point is selected and its placed on top of game object)
         if (m_movementOnEditorObject)
         {
            m_currentEditorObjectSelected->Rotate(-angle, true);
            m_gui.ObjectUpdated(m_currentEditorObjectSelected->GetLinkedObject()->GetID());
         }
         else
         {
            m_currentSelectedGameObject->Rotate(-angle, true);
            m_gui.ObjectUpdated(m_currentSelectedGameObject->GetID());
         }
      }
      else
      {
         m_camera.Rotate(angle);
      }
   }
   // Move camera (or currently selected Object)
   else
   {
      auto mouseMovementLength = glm::length(currentCursorPos - m_lastCursorPosition);

      const auto minCameraMovement = 1.0f;
      const auto maxCameraMovement = 2.0f;

      mouseMovementLength = glm::clamp(mouseMovementLength, minCameraMovement, maxCameraMovement);

      const auto moveBy = glm::vec3(-axis.x, -axis.y, 0.0f);

      if (m_movementOnEditorObject || m_movementOnGameObject)
      {
         // Editor objects selected have higher priority of movement
         // for example when animation point is selected and its placed on top of game object)
         if (m_movementOnEditorObject)
         {
            m_currentEditorObjectSelected->Move(m_camera.ConvertToCameraVector(-moveBy), false);
            m_gui.ObjectUpdated(m_currentEditorObjectSelected->GetLinkedObject()->GetID());
         }
         else
         {
            m_currentSelectedGameObject->Move(m_camera.ConvertToCameraVector(-moveBy), false);
            m_gui.ObjectUpdated(m_currentSelectedGameObject->GetID());
         }
      }
      else
      {
         m_camera.Move(moveBy);
      }
   }

   m_mouseDrag = true;
}

bool
Editor::mouseMotionEvent(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers)
{
   if (!Screen::mouseMotionEvent(p, rel, button, modifiers))
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
   }

   return true;
}

void
Editor::HandleGameObjectSelected(std::shared_ptr< GameObject > newSelectedGameObject)
{
   if (m_currentSelectedGameObject != newSelectedGameObject)
   {
      if (m_currentSelectedGameObject)
      {
         // unselect previously selected object
         m_currentSelectedGameObject->SetObjectUnselected();
      }

      m_currentSelectedGameObject = newSelectedGameObject;

      // mark new object as selected
      m_currentSelectedGameObject->SetObjectSelected();
      m_gameObjectSelected = true;

      if (m_editorObjectSelected)
      {
         UnselectEditorObject();
      }

      m_gui.GameObjectSelected(m_currentSelectedGameObject);
   }

   m_movementOnGameObject = true;
}

void
Editor::HandleEditorObjectSelected(std::shared_ptr< EditorObject > newSelectedEditorObject)
{
   if (m_editorObjectSelected && (newSelectedEditorObject != m_currentEditorObjectSelected))
   {
      UnselectEditorObject();
   }

   m_currentEditorObjectSelected = newSelectedEditorObject;
   m_editorObjectSelected = true;
   m_movementOnEditorObject = true;
   m_currentEditorObjectSelected->Scale({2.0f, 2.0f});
}

void
Editor::UnselectEditorObject()
{
   m_editorObjectSelected = false;
   m_movementOnEditorObject = false;
   m_currentEditorObjectSelected->Scale({1.0f, 1.0f});
}

void
Editor::CheckIfObjectGotSelected(const glm::vec2& cursorPosition)
{
   auto newSelectedEditorObject = std::find_if(m_editorObjects.begin(), m_editorObjects.end(), [cursorPosition](auto& object) {
      return object->CheckIfCollidedScreenPosion(cursorPosition);
   });

   if (newSelectedEditorObject != m_editorObjects.end())
   {
      HandleEditorObjectSelected(*newSelectedEditorObject);
   }

   auto newSelectedObject = m_currentLevel.GetGameObjectOnLocation(cursorPosition);

   if (newSelectedObject)
   {
      HandleGameObjectSelected(newSelectedObject);
   }
}

bool
Editor::mouseButtonEvent(const nanogui::Vector2i& position, int button, bool down, int modifiers)
{
   if (!Screen::mouseButtonEvent(position, button, down, modifiers))
   {
      m_mousePressedLastUpdate = down;

      if (down)
      {
         CheckIfObjectGotSelected({position.x(), position.y()});
      }
      else
      {
         // Object movement finished
         ShowCursor(true);

         if (m_mouseDrag && (m_movementOnEditorObject || m_movementOnGameObject))
         {
            if (m_movementOnEditorObject)
            {
               const auto cursporPos = m_currentEditorObjectSelected->GetScreenPositionPixels();
               glfwSetCursorPos(mGLFWWindow, cursporPos.x, cursporPos.y);
            }
            else if (m_movementOnGameObject)
            {
               const auto cursporPos = m_currentSelectedGameObject->GetScreenPositionPixels();
               glfwSetCursorPos(mGLFWWindow, cursporPos.x, cursporPos.y);
            }
         }

         m_movementOnEditorObject = false;
         m_movementOnGameObject = false;
         m_mouseDrag = false;
      }
   }

   return true;
}

bool
Editor::keyboardEvent(int key, int scancode, int action, int modifiers)
{
   if (!Screen::keyboardEvent(key, scancode, action, modifiers))
   {
      m_keyMap[key] = action;
   }

   return true;
}

void
Editor::drawAll()
{
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

      for (auto& object : m_editorObjects)
      {
         if (object->GetVisible())
         {
            object->Render(m_currentLevel.GetShader());
         }
      }
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
   m_camera.SetLevelSize(m_currentLevel.GetSize());

   m_levelLoaded = true;
   m_gui.LevelLoaded(&m_currentLevel);
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

   // Populate editor objects
   const auto gameObjects = m_currentLevel.GetObjects();
   for (const auto& object : gameObjects)
   {
      const auto animatablePtr = std::dynamic_pointer_cast< Animatable >(object);

      if (animatablePtr)
      {
         auto animationPoints = animatablePtr->GetAnimationKeypoints();

         for (auto& point : animationPoints)
         {
            auto editorObject = std::make_shared< EditorObject >(*this, point->m_end, glm::ivec2(20, 20), "Default128.png",
                                                                 std::dynamic_pointer_cast<::Object >(point));
            editorObject->SetName("Animationpoint" + object->GetName());

            m_editorObjects.push_back(editorObject);
            m_objects.push_back(std::dynamic_pointer_cast<::Object >(point));
         }
      }
   }

   m_camera.Create(glm::vec3(m_currentLevel.GetPlayer()->GetCenteredGlobalPosition(), 0.0f), {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f});
   m_camera.SetLevelSize(m_currentLevel.GetSize());

   m_levelLoaded = true;
   m_gui.LevelLoaded(&m_currentLevel);
}

void
Editor::SaveLevel(const std::string& levelPath)
{
   m_currentLevel.Save(levelPath);
}

void
Editor::AddGameObject(GameObject::TYPE objectType)
{
   HandleGameObjectSelected(m_currentLevel.AddGameObject(objectType));
}

void
Editor::ToggleAnimateObject()
{
   if (m_animateGameObject)
   {
      auto enemyPtr = std::dynamic_pointer_cast< Enemy >(m_currentSelectedGameObject);
      enemyPtr->SetLocalPosition(enemyPtr->GetInitialPosition());
      m_animateGameObject = false;
   }
   else
   {
      m_animateGameObject = true;
   }
}

bool
Editor::IsObjectAnimated()
{
   return m_animateGameObject;
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
Editor::ShowWireframe(bool wireframeEnabled)
{
   for (auto& object : m_currentLevel.GetObjects())
   {
      wireframeEnabled ? object->SetObjectSelected() : object->SetObjectUnselected();
   }

   if (m_player)
   {
      wireframeEnabled ? m_currentLevel.GetPlayer()->SetObjectSelected() : m_currentLevel.GetPlayer()->SetObjectUnselected();
   }

   // Make sure currenlty selected object stays selected
   if (m_currentSelectedGameObject)
   {
      m_currentSelectedGameObject->SetObjectSelected();
   }
}

void
Editor::SetRenderAnimationPoints(bool render)
{
   const auto animatablePtr = std::dynamic_pointer_cast< Animatable >(m_currentSelectedGameObject);

   if (animatablePtr)
   {
      animatablePtr->RenderAnimationSteps(render);
      auto animationPoints = animatablePtr->GetAnimationKeypoints();

      const auto currenltySelectedName = m_currentSelectedGameObject->GetName();

      std::for_each(m_editorObjects.begin(), m_editorObjects.end(), [render, currenltySelectedName](auto& object) {
         if (object->GetName() == "Animationpoint" + currenltySelectedName)
         {
            object->SetVisible(render);
         }
      });
   }
}

void
Editor::SetLockAnimationPoints(bool lock)
{
   const auto animatablePtr = std::dynamic_pointer_cast< Animatable >(m_currentSelectedGameObject);

   if (animatablePtr)
   {
      animatablePtr->LockAnimationSteps(lock);
      /*   auto animationPoints = animatablePtr->GetAnimationKeypoints();

         const auto currenltySelectedName = m_currentSelectedObject->GetName();

         std::for_each(m_editorObjects.begin(), m_editorObjects.end(), [render, currenltySelectedName](EditorObject& object) {
            if (object.GetName() == "Animationpoint" + currenltySelectedName)
            {
               object.SetVisible(render);
            }
         });*/
   }
}

void
Editor::Update()
{
   if (m_animateGameObject && m_currentSelectedGameObject)
   {
      auto moveBy = std::dynamic_pointer_cast< Animatable >(m_currentSelectedGameObject)->SingleAnimate(m_deltaTime);

      if (moveBy.has_value())
      {
         m_currentSelectedGameObject->Move(moveBy.value(), false);
         //m_currentSelectedGameObject->GetSprite().SetTranslateValue(moveBy.value());
      }
      else
      {
         m_animateGameObject = false;
      }
   }
}

const glm::vec2&
Editor::GetWindowSize() const
{
   return m_windowSize;
}

const glm::ivec2&
Editor::GetFrameBufferwSize() const
{
   int viewportWidth, viewportHeight;
   glfwGetFramebufferSize(mGLFWWindow, &viewportWidth, &viewportHeight);

   return {viewportWidth, viewportHeight};
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

bool
Editor::IsKeyDown(uint8_t keycode)
{
   return m_keyMap[keycode];
}

void
Editor::MainLoop()
{
   Logger::SetLogType(Logger::TYPE::INFO);

   while (IsRunning())
   {
      PollEvents();

      // override keyboard input
      HandleInput();

      // Update UI
      m_gui.Update();
      Update();

      drawAll();
   }
}
