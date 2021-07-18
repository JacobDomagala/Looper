#pragma once

#include "Application.hpp"
#include "EditorGUI.hpp"
#include "EditorObject.hpp"
#include "Game.hpp"
#include "Level.hpp"
#include "Logger.hpp"
#include "Object.hpp"
#include "Player.hpp"
#include "Timer.hpp"

#include <glm/matrix.hpp>
#include <utility>

namespace dgame {

class Player;
class Window;

class Editor : public Application
{
 public:
   Editor(const glm::ivec2& screenSize);
   ~Editor() override = default;

   // APPLICATION OVERRIDES
   void
   MainLoop() override;

   glm::vec2
   GetWindowSize() const override;

   const glm::mat4&
   GetProjection() const override;

   const glm::mat4&
   GetViewMatrix() const override;

   float
   GetZoomLevel() override;

   void
   KeyCallback(const KeyEvent& event) override;

   void
   MouseButtonCallback(const MouseButtonEvent& event) override;

   void
   CursorPositionCallback(const CursorPositionEvent& event) override;

   void
   MouseScrollCallback(const MouseScrollEvent& event) override;

   // EDITOR SPECIFIC FUNCTIONS
   void
   Render();

   void
   CreateLevel(const glm::ivec2& size);

   void
   LoadLevel(const std::string& levelName);

   void
   SaveLevel(const std::string& levelName);

   void
   AddGameObject(GameObject::TYPE objectType);

   void
   AddObject(Object::TYPE objectType);

   void
   ToggleAnimateObject();

   bool
   IsObjectAnimated();

   void
   PlayLevel();

   void
   LaunchGameLoop();

   void
   RenderNodes(bool render);

   void
   SetRenderAnimationPoints(bool render);

   void
   SetLockAnimationPoints(bool render);

   void
   Update();

   void
   UpdateAnimationData();

   bool
   GetRenderNodes() const;

   void
   DrawGrid();

   void
   SetGridData(bool render, int32_t cellSize);

   std::pair< bool, int32_t >
   GetGridData() const;

   void
   HandleGameObjectSelected(std::shared_ptr< GameObject > newSelectedGameObject,
                            bool fromGUI = false);

   void
   HandleObjectSelected(Object::ID objectID, bool fromGUI);

   void
   HandleEditorObjectSelected(std::shared_ptr< EditorObject > newSelectedObject,
                              bool fromGUI = false);

 private:
   enum class ACTION
   {
      UNSELECT,
      REMOVE
   };

 private:
   std::shared_ptr< EditorObject >
   GetEditorObjectByID(Object::ID ID);

   void
   ActionOnObject(ACTION action);

   void
   DrawBackgroundObjects();

   void
   DrawEditorObjects();

   void
   DrawAnimationPoints();

   void
   DrawBoundingBoxes();

   bool
   IsRunning() override;

   void
   HandleCamera();

   void
   HandleMouseDrag(const glm::vec2& currentCursorPos, const glm::vec2& axis);

   void
   CheckIfObjectGotSelected(const glm::vec2& cursorPosition);

   void
   UnselectEditorObject();

   void
   SelectGameObject();

   void
   UnselectGameObject();

   void
   ShowCursor(bool choice);

   void
   SetMouseOnObject();

   std::unique_ptr< Game > m_game;
   std::shared_ptr< Level > m_level;

   std::string m_levelFileName;

   bool m_isRunning = true;
   bool m_levelLoaded = false;
   bool m_mousePressedLastUpdate = false;
   bool m_mouseDrag = false;

   glm::vec2 m_lastCursorPosition;

   std::unique_ptr< byte_vec4 > m_collision = nullptr;

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

   bool m_renderPathfinderNodes = true;

   bool m_drawGrid = true;
   int32_t m_gridCellSize = 128;

   EditorGUI m_gui;

   bool m_playGame = false;
};

} // namespace dgame