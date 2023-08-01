#pragma once

#include "application.hpp"
#include "editor_object.hpp"
#include "game.hpp"
#include "gui/editor_gui.hpp"
#include "level.hpp"
#include "logger.hpp"
#include "object.hpp"
#include "player.hpp"
#include "utils/time/time_step.hpp"
#include "thread_pool.hpp"

#include <glm/matrix.hpp>
#include <utility>

namespace looper {

class Player;
class Animatable;

class Editor : public Application
{
 public:
   enum class ACTION
   {
      UNSELECT,
      REMOVE
   };
   explicit Editor(const glm::ivec2& screenSize);

   // APPLICATION OVERRIDES
   void
   MainLoop() override;

   [[nodiscard]] glm::vec2
   GetWindowSize() const override;

   [[nodiscard]] const glm::mat4&
   GetProjection() const override;

   [[nodiscard]] const glm::mat4&
   GetViewMatrix() const override;

   [[nodiscard]] float
   GetZoomLevel() const override;

   void
   KeyCallback(const KeyEvent& event) override;

   void
   MouseButtonCallback(const MouseButtonEvent& event) override;

   void
   CursorPositionCallback(const CursorPositionEvent& event) override;

   void
   MouseScrollCallback(const MouseScrollEvent& event) override;

   void
   Render(VkCommandBuffer cmdBuffer) override;

   // EDITOR SPECIFIC FUNCTIONS
   void
   CreateLevel(const std::string& name, const glm::ivec2& size);

   void
   LoadLevel(const std::string& levelPath);

   void
   SaveLevel(const std::string& levelPath);

   void
   AddGameObject(ObjectType objectType);

   void
   CopyGameObject(const std::shared_ptr< GameObject >& objectToCopy);

   void
   AddObject(ObjectType objectType);

   void
   ToggleAnimateObject();

   [[nodiscard]] bool
   IsObjectAnimated() const;

   void
   PlayLevel();

   void
   LaunchGameLoop();

   void
   RenderNodes(bool render);

   void
   SetRenderAnimationPoints(bool render);

   void
   SetLockAnimationPoints(bool lock);

   void
   Update();

   void
   UpdateAnimationData();

   [[nodiscard]] bool
   GetRenderNodes() const;

   void
   DrawGrid();

   void
   DrawBoundingBoxes();

   void
   SetGridData(bool render, int32_t cellSize);

   [[nodiscard]] std::pair< bool, int32_t >
   GetGridData() const;

   [[nodiscard]] time::TimeStep
   GetFrameTime() const;

   [[nodiscard]] time::TimeStep
   GetUpdateUITime() const;

   [[nodiscard]] time::TimeStep
   GetRenderTime() const;

   [[nodiscard]] std::pair< uint32_t, uint32_t >
   GetRenderOffsets() const;

   void
   HandleGameObjectSelected(const std::shared_ptr< GameObject >& newSelectedGameObject,
                            bool fromGUI = false);

   void
   HandleObjectSelected(Object::ID objectID, bool fromGUI);

   [[nodiscard]] Object::ID
   GetSelectedEditorObject() const;

   void
   HandleEditorObjectSelected(const std::shared_ptr< EditorObject >& newSelectedEditorObject,
                              bool fromGUI = false);

   void
   ActionOnObject(ACTION action);

 private:
   // [[nodiscard]] std::shared_ptr< EditorObject >
   // GetEditorObjectByID(Object::ID ID);

   void
   SetupRendererData();

   void
   DrawEditorObjects();

   void
   DrawAnimationPoints();

   void
   SetVisibleAnimationPoints(const std::shared_ptr< Animatable >& animatablePtr, bool visible);

   // void
   // DrawBoundingBoxes();

   [[nodiscard]] bool
   IsRunning() const override;

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

   void
   SetupPathfinderNodes();

   void
   FreeLevelData();

   std::unique_ptr< Game > m_game = {};

   std::string m_levelFileName = {};

   bool m_isRunning = true;
   bool m_levelLoaded = false;
   bool m_mousePressedLastUpdate = false;
   bool m_mouseDrag = false;

   glm::vec2 m_lastCursorPosition = {};

   // Handling game objects (which are visible in game)
   bool m_animateGameObject = false;
   bool m_movementOnGameObject = false;
   bool m_gameObjectSelected = false;
   std::shared_ptr< GameObject > m_currentSelectedGameObject = {};

   std::shared_ptr< GameObject > m_copiedGameObject = {};

   // Handling of editor objects (drawable objects linked to object in game)
   bool m_movementOnEditorObject = false;
   bool m_editorObjectSelected = false;
   std::vector< std::shared_ptr< EditorObject > > m_editorObjects = {};
   std::vector< std::shared_ptr< EditorObject > > pathfinderNodes_ = {};
   std::vector< std::shared_ptr< EditorObject > > animationPoints_ = {};
   std::shared_ptr< EditorObject > m_currentEditorObjectSelected = {};

   bool m_renderPathfinderNodes = false;

   uint32_t numPathfinderNodes_ = {};

   bool m_drawGrid = false;
   int32_t m_gridCellSize = 128;

   // constructed in initializer list
   EditorGUI gui_;

   bool m_playGame = false;
   time::TimeStep timeLastFrame_ = time::TimeStep{time::microseconds{}};
   time::TimeStep uiTime_ = time::TimeStep{time::microseconds{}};
   time::TimeStep renderTime_ = time::TimeStep{time::microseconds{}};

   ThreadPool pool_ = ThreadPool{std::thread::hardware_concurrency()};
   std::future< void > updateReady_;
   std::future< void > renderReady_;
};

} // namespace looper
