#pragma once

#include "Context.hpp"
#include "EditorObject.hpp"
#include "Game.hpp"
#include "Gui.hpp"
#include "Level.hpp"
#include "Logger.hpp"
#include "Player.hpp"
#include "Timer.hpp"
#include "Object.hpp"

#include <glm/matrix.hpp>
#include <nanogui/screen.h>
#include <utility>

class Player;
class Window;

class Editor : public nanogui::Screen, public Context
{
 public:
   Editor(const glm::vec2& screenSize);
   ~Editor();

   // CONTEXT OVERRIDES
   void
   MainLoop() override;

   const glm::vec2&
   GetWindowSize() const override;

   const glm::ivec2&
   GetFrameBufferwSize() const override;

   const glm::mat4&
   GetProjection() const override;

   const glm::mat4&
   GetViewMatrix() const override;

   float
   GetZoomLevel() override;


   // NANOGUI::SCREEN OVERRIDES
   void
   draw(NVGcontext* ctx) override;

   void
   drawAll() override;

   void
   drawContents() override;

   bool
   scrollEvent(const nanogui::Vector2i& p, const nanogui::Vector2f& rel) override;

   bool
   mouseMotionEvent(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers) override;

   bool
   mouseButtonEvent(const nanogui::Vector2i& p, int button, bool down, int modifiers) override;

   bool
   keyboardEvent(int key, int scancode, int action, int modifiers) override;


   // EDITOR SPECIFIC FUNCTIONS
   void
   CreateLevel(const glm::ivec2& size);

   void
   LoadLevel(const std::string& levelName);

   void
   SaveLevel(const std::string& levelName);

   void
   AddGameObject(GameObject::TYPE objectType);

   void
   AnimateObject(float value);

   void
   PlayLevel();

   void
   ShowWireframe(bool wireframeEnabled);

   void
   SetRenderAnimationPoints(bool render);

   void
   SetLockAnimationPoints(bool render);

   void
   Update();

 private:
   bool
   IsRunning() override;

   void
   InitGLFW();

   void
   HandleInput();

   void
   HandleMouseDrag(const glm::vec2& currentCursorPos, const glm::vec2& axis);

   void
   CheckIfObjectGotSelected(const glm::vec2& cursorPosition);

   void
   HandleObjectSelected(std::shared_ptr< GameObject > newSelectedObject);

   void
   ShowCursor(bool choice);

   bool
   IsKeyDown(uint8_t keycode);

   Game m_game;

   std::string m_levelFileName;

   bool m_isRunning = true;
   bool m_levelLoaded = false;
   bool m_mousePressedLastUpdate = false;
   bool m_mouseDrag = false;

   glm::vec2 m_lastCursorPosition;

   std::unique_ptr< byte_vec4 > m_collision = nullptr;

   // projection matrix for OpenGL
   glm::mat4 m_projectionMatrix;
   glm::vec2 m_windowSize;

   float m_zoomScale = 0.0f;
   float m_maxZoomIn = 1.5f;
   float m_maxZoomOut = -1.5f;

   bool m_animateObject = false;
   bool m_movementOnObject = false;
   bool m_objectSelected = false;
   std::shared_ptr< GameObject > m_currentSelectedObject;

   bool m_editorObjectSelected = false;
   std::vector< EditorObject > m_editorObjects;
   std::vector< EditorObject >::iterator m_currentEditorObjectSelected = m_editorObjects.end(); 

   std::list< std::shared_ptr<::Object> > m_objects;

   std::map< uint8_t, bool > m_keyMap;
   Gui m_gui;
};
