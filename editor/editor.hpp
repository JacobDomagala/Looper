#pragma once

#include "application.hpp"
#include "editor_object.hpp"
#include "gui/editor_gui.hpp"
#include "level.hpp"
#include "logger.hpp"
#include "object.hpp"
#include "player.hpp"
#include "thread_pool.hpp"
#include "utils/time/time_step.hpp"

#include <glm/matrix.hpp>
#include <optional>
#include <utility>

namespace looper {

class Player;
class Animatable;
struct AnimationPoint;

class Editor : public Application
{
 public:
   enum class ACTION
   {
      UNSELECT,
      REMOVE,
      NONE
   };
   explicit Editor(const glm::ivec2& screenSize);

   // APPLICATION OVERRIDES
   void
   MainLoop() override;

   void
   SelectAnimationPoint(const AnimationPoint& node);

   void
   AddAnimationPoint(const glm::vec2& position);

   [[nodiscard]] glm::vec2
   GetWindowSize() const override;

   [[nodiscard]] const glm::mat4&
   GetProjection() const override;

   [[nodiscard]] const glm::mat4&
   GetViewMatrix() const override;

   [[nodiscard]] float
   GetZoomLevel() const override;

   void
   KeyCallback(KeyEvent& event) override;

   void
   MouseButtonCallback(MouseButtonEvent& event) override;

   void
   CursorPositionCallback(CursorPositionEvent& event) override;

   void
   MouseScrollCallback(MouseScrollEvent& event) override;

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
   AddGameObject(ObjectType objectType, const glm::vec2& position);

   void
   CopyGameObjects(const std::vector< Object::ID >& objectsToCopy);

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
   UpdateAnimationData(Object::ID object);

   [[nodiscard]] bool
   GetRenderNodes() const;

   void
   DrawGrid() const;

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
   HandleGameObjectSelected(Object::ID newSelectedGameObject, bool groupSelect, bool fromGUI = false);

   void
   HandleObjectSelected(Object::ID objectID, bool fromGUI);

   [[nodiscard]] Object::ID
   GetSelectedEditorObject();

   [[nodiscard]] Object::ID
   GetSelectedGameObject() const;

   const std::vector< Object::ID >&
   GetSelectedObjects() const;

   void
   HandleEditorObjectSelected(EditorObject& newSelectedEditorObject,
                              bool fromGUI = false);

   void
   ActionOnObject(ACTION action, Object::ID object);

   void
   AddToWorkQueue(
      const WorkQueue::WorkUnit& work, const WorkQueue::Precondition& prec = [] { return true; });

   template < class F, class... Args >
   auto
   AddToThreadPool(F&& f, Args&&... args)
   {
      return threadPool_.enqueue(std::forward< F >(f), std::forward< Args >(args)...);
   }

   void
   Shutdown();

   bool
   IsAnyObjectSelected() const;

   int32_t
   GetRenderLayerToDraw() const;

   void SetRenderLayerToDraw(int32_t);

   EditorObject&
   GetEditorObjectRef(Object::ID object);

 private:
   // [[nodiscard]] std::shared_ptr< EditorObject >
   // GetEditorObjectByID(Object::ID ID);

   void
   SetupRendererData() const;

   void
   DrawAnimationPoints();

   void
   SetVisibleAnimationPoints(const Animatable& animatable, bool visible);

   // void
   // DrawBoundingBoxes();

   [[nodiscard]] bool
   IsRunning() const override;

   void
   HandleCamera();

   void
   HandleMouseDrag(const glm::vec2& currentCursorPos, const glm::vec2& axis);

   void
   RotateLogic(const glm::vec2& currentCursorPos);

   void
   MoveLogic(const glm::vec2& axis);

   void
   CheckIfObjectGotSelected(const glm::vec2& cursorPosition, bool groupSelect);

   std::vector< Object::ID >
   GetObjectsInArea(const std::array< glm::vec2, 4 >& area) const;

   void
   UnselectEditorObject(Object::ID object);

   void
   SelectGameObject();

   void
   UnselectGameObject(Object::ID object, bool groupSelect);

   void
   ShowCursor(bool choice);

   void
   SetMouseOnObject();

   void
   FreeLevelData();

   std::string m_levelFileName = {};

   bool isRunning_ = true;
   bool levelLoaded_ = false;
   bool shouldUpdateRenderer_ = false;

   // Left and right mouse buttons
   bool LMBPressedLastUpdate_ = false;
   bool RMBPressedLastUpdate_ = false;

   bool mouseDrag_ = false;
   bool selectingObjects_ = false;

   glm::vec2 selectStartPos_ = {};
   std::array< glm::vec2, 4 > selectRect_ = {};
   std::vector< Object::ID > selectedObjects_ = {};

   glm::vec2 lastCursorPosition_ = {};

   // Handling game objects (which are visible in game)
   bool animateGameObject_ = false;
   bool movementOnGameObject_ = false;
   bool gameObjectSelected_ = false;
   Object::ID currentSelectedGameObject_ = Object::INVALID_ID;

   std::vector< Object::ID > copiedGameObjects_ = {};

   // Handling of editor objects (drawable objects linked to object in game)
   bool movementOnEditorObject_ = false;
   bool editorObjectSelected_ = false;
   std::vector< EditorObject > pathfinderNodes_ = {};
   std::vector< EditorObject > animationPoints_ = {};
   Object::ID currentEditorObjectSelected_ = Object::INVALID_ID;

   bool renderPathfinderNodes_ = false;

   uint32_t numPathfinderNodes_ = {};

   bool drawGrid_ = false;
   int32_t gridCellSize_ = 128;
   int32_t renderLayerToDraw_ = -1;

   // constructed in initializer list
   EditorGUI gui_;

   bool playGame_ = false;
   time::TimeStep timeLastFrame_ = time::TimeStep{time::microseconds{}};
   time::TimeStep uiTime_ = time::TimeStep{time::microseconds{}};
   time::TimeStep renderTime_ = time::TimeStep{time::microseconds{}};

   ThreadPool threadPool_ = ThreadPool{std::thread::hardware_concurrency()};
   std::future< void > updateReady_;
   std::future< void > renderReady_;
};

} // namespace looper
