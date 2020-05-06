#pragma once

#include "Application.hpp"
#include "EditorObject.hpp"
#include "Game.hpp"
#include "Gui.hpp"
#include "Level.hpp"
#include "Logger.hpp"
#include "Object.hpp"
#include "Player.hpp"
#include "Timer.hpp"

#include <glm/matrix.hpp>
#include <nanogui/screen.h>
#include <utility>

namespace dgame {

class Player;
class Window;

class Editor : public nanogui::Screen, public Application
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
   ToggleAnimateObject();

   bool
   IsObjectAnimated();

   void
   PlayLevel();

   void
   ShowWireframe(bool wireframeEnabled);

   void
   ShowWaypoints(bool wireframeEnabled);

   void
   SetRenderAnimationPoints(bool render);

   void
   SetLockAnimationPoints(bool render);

   void
   Update();

   void
   UpdateAnimationData();

 private:
   void
   DrawEditorObjects();

   void
   DrawAnimationPoints();

   bool
   IsRunning() override;

   void
   InitGLFW();

   void
   HandleCamera();

   void
   HandleMouseDrag(const glm::vec2& currentCursorPos, const glm::vec2& axis);

   void
   CheckIfObjectGotSelected(const glm::vec2& cursorPosition);

   void
   HandleGameObjectSelected(std::shared_ptr< GameObject > newSelectedGameObject);

   void
   HandleEditorObjectSelected(std::shared_ptr< EditorObject > newSelectedObject);

   void
   UnselectEditorObject();

   void
   ShowCursor(bool choice);

   bool
   IsKeyDown(uint8_t keycode);

   std::unique_ptr< Game > m_game;
   std::shared_ptr< Level > m_level;

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

   // Handling game objects (which are visible in game)
   bool m_animateGameObject = false;
   bool m_movementOnGameObject = false;
   bool m_gameObjectSelected = false;
   std::shared_ptr< GameObject > m_currentSelectedGameObject;

   // Handling of editor objects (drawable objects linked to object in game)
   bool m_movementOnEditorObject = false;
   bool m_editorObjectSelected = false;
   std::vector< std::shared_ptr< EditorObject > > m_editorObjects;
   std::shared_ptr< EditorObject > m_currentEditorObjectSelected;

   // Represents all objects located in game, such as Gameobjects, light sources, particle emiters etc.
   std::vector< std::shared_ptr<Object > > m_objects;

   bool m_showWaypoints = true;
   std::map< uint8_t, bool > m_keyMap;
   Gui m_gui;
};

} // namespace dgame