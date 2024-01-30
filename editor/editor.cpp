#include "editor.hpp"
#include "enemy.hpp"
#include "game.hpp"
#include "input_manager.hpp"
#include "renderer/renderer.hpp"
#include "renderer/shader.hpp"
#include "renderer/sprite.hpp"
#include "renderer/vulkan_common.hpp"
#include "renderer/window/window.hpp"
#include "utils/file_manager.hpp"
#include "utils/time/scoped_timer.hpp"
#include "utils/time/stopwatch.hpp"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.h>

#include <memory>
#include <set>
#include <string>

namespace looper {

Editor::Editor(const glm::ivec2& screenSize) : gui_(*this)
{
   window_.Init(screenSize, "Editor", true);

   InputManager::Init(window_.GetWindowHandle());

   renderer::Initialize(window_.GetWindowHandle(), renderer::ApplicationType::EDITOR);
   gui_.Init();
   InputManager::RegisterForInput(window_.GetWindowHandle(), this);
}

void
Editor::ShowCursor(bool choice)
{
   window_.ShowCursor(choice);
}

void
Editor::KeyCallback(KeyEvent& event)
{
   if (event.action_ == GLFW_PRESS)
   {
      if (gizmoActive_)
      {
         if (event.key_ == GLFW_KEY_T)
         {
            gizmo_.SwitchToTranslate();
         }
         if (event.key_ == GLFW_KEY_R)
         {
            gizmo_.SwitchToRotate();
         }
         if (event.key_ == GLFW_KEY_S)
         {
            gizmo_.SwitchToScale();
         }
      }

      if (IsAnyObjectSelected())
      {
         ACTION action = ACTION::NONE;

         if (event.key_ == GLFW_KEY_ESCAPE)
         {
            action = ACTION::UNSELECT;
         }
         if (event.key_ == GLFW_KEY_DELETE)
         {
            action = ACTION::REMOVE;
         }

         if (action != ACTION::NONE)
         {
            if (currentSelectedEditorObject_ != Object::INVALID_ID)
            {
               ActionOnObject(action, currentSelectedEditorObject_);
            }
            else
            {
               for (auto it = selectedObjects_.begin(); it < selectedObjects_.end();)
               {
                  ActionOnObject(action, *it);
                  // 'ActionOnObject' will delete an entry in 'selectedObjects_' we have to
                  // re-assign iterator here
                  it = selectedObjects_.begin();
               }
            }

            event.handled_ = true;
         }
      }
   }
   else if (event.action_ == GLFW_RELEASE)
   {
      if (InputManager::CheckKeyPressed(GLFW_KEY_LEFT_CONTROL))
      {
         if (event.key_ == GLFW_KEY_C)
         {
            copiedGameObjects_ = selectedObjects_;
            event.handled_ = true;
         }
         else if (event.key_ == GLFW_KEY_V)
         {
            CopyGameObjects(copiedGameObjects_);
            event.handled_ = true;
         }
      }
   }
}

void
Editor::MouseScrollCallback(MouseScrollEvent& event)
{
   if (!playGame_ && !EditorGUI::IsBlockingEvents() && levelLoaded_)
   {
      camera_.Zoom(static_cast< float >(event.xOffset_ + event.yOffset_));
      event.handled_ = true;
   }
}

void
Editor::MouseButtonCallback(MouseButtonEvent& event)
{
   if (playGame_ or EditorGUI::IsBlockingEvents() or not levelLoaded_)
   {
      return;
   }

   const auto mousePressed = event.action_ == GLFW_PRESS;
   const auto mouseReleased = event.action_ == GLFW_RELEASE;
   LMBPressedLastUpdate_ = mousePressed and event.button_ == GLFW_MOUSE_BUTTON_1;
   RMBPressedLastUpdate_ = mousePressed and event.button_ == GLFW_MOUSE_BUTTON_2;

   if (mouseReleased and not mouseDrag_)
   {
      CheckIfObjectGotSelected(InputManager::GetMousePos(),
                               InputManager::CheckKeyPressed(GLFW_KEY_LEFT_CONTROL));
   }
   else if (movementOnEditorObject_ or movementOnGameObject_ or selectingObjects_)
   {
      if (movementOnGameObject_ or movementOnEditorObject_)
      {
         // Object movement finished
         ShowCursor(true);
         SetMouseOnObject();
      }

      movementOnEditorObject_ = false;
      movementOnGameObject_ = false;

      if (selectingObjects_)
      {
         const auto selectedObjects = GetObjectsInArea(selectRect_);

         if (not selectedObjects.empty())
         {
            if (currentSelectedGameObject_ != Object::INVALID_ID)
            {
               ActionOnObject(ACTION::UNSELECT, currentSelectedGameObject_);
            }

            for (const auto& object : selectedObjects_)
            {
               gui_.ObjectUnselected(object);
            }


            auto& firstObject = currentLevel_->GetGameObjectRef(selectedObjects.front());
            auto gizmoPos = firstObject.GetCenteredPosition();
            glm::vec2 min = gizmoPos;
            glm::vec2 max = gizmoPos;

            for (const auto object : selectedObjects)
            {
               gui_.ObjectSelected(object);
               const auto& objectPos =
                  currentLevel_->GetGameObjectRef(object).GetCenteredPosition();

               min = glm::vec2{
                  glm::min(min.x, objectPos.x),
                  glm::min(min.y, objectPos.y),
               };

               max = glm::vec2{
                  glm::max(max.x, objectPos.x),
                  glm::max(max.y, objectPos.y),
               };
            }

            selectedObjects_ = selectedObjects;

            gizmoActive_ = true;
            gizmo_.Show();
            gizmo_.Update((min + max) / 2.0f, selectedObjects_.size() == 1
                                                 ? firstObject.GetSprite().GetRotation()
                                                 : 0.0f);
         }
         selectStartPos_ = glm::vec2{};
         selectRect_ = std::array< glm::vec2, 4 >{};
      }
      selectingObjects_ = false;
   }

   mouseDrag_ = false;
   event.handled_ = true;
}

void
Editor::CursorPositionCallback(CursorPositionEvent& event)
{
   if (!playGame_ && !EditorGUI::IsBlockingEvents() && levelLoaded_)
   {
      const auto currentCursorPosition = glm::vec2(event.xPos_, event.yPos_);

      if (RMBPressedLastUpdate_ or LMBPressedLastUpdate_)
      {
         HandleMouseDrag(currentCursorPosition, currentCursorPosition - lastCursorPosition_);
      }
      else
      {
         if (gizmoActive_)
         {
            gizmo_.CheckHovered(camera_.GetPosition(), ScreenToGlobal(currentCursorPosition));
         }

         ShowCursor(true);
      }

      lastCursorPosition_ = currentCursorPosition;
      event.handled_ = true;
   }
}

void
Editor::RotateLogic(const glm::vec2& currentCursorPos)
{
   // Calculate the value of cursor movement
   // For example:
   // - cursor was moved to the right then movementVector.x is positive, negative otherwise
   // - cursor was moved to the top of window then movementVector.y is positive, negative
   // otherwise
   const auto movementVector = currentCursorPos - lastCursorPosition_;

   // Compute the bigger factor (magnitute hence abs())
   // Go with `-movementVector.x` because it feels right
   const auto movementVal = std::abs(movementVector.x) > std::abs(movementVector.y)
                               ? -movementVector.x
                               : movementVector.y;

   constexpr auto maxRotationAngle = 0.02f;

   const auto angle = glm::clamp(movementVal, -maxRotationAngle, maxRotationAngle);


   // Editor objects selected have higher priority of movement
   // for example when animation point is selected and it's placed on top of game object
   if (currentSelectedEditorObject_ != Object::INVALID_ID)
   {
      auto& selectedEditorObject = GetEditorObjectRef(currentSelectedEditorObject_);
      selectedEditorObject.Rotate(-angle, true);
      gui_.ObjectUpdated(selectedEditorObject.GetLinkedObjectID());
   }
   else if (currentSelectedGameObject_ != Object::INVALID_ID)
   {
      auto& object = currentLevel_->GetGameObjectRef(currentSelectedGameObject_);
      object.Rotate(-angle, true);
      gui_.ObjectUpdated(currentSelectedGameObject_);
      gizmo_.Update(object.GetCenteredPosition(), object.GetSprite().GetRotation());
   }
}

void
Editor::MoveLogic(const glm::vec2& axis)
{
   auto moveBy = glm::vec3(axis, 0.0f);

   // By default we apply movement on both axis
   if (gizmo_.selectedPart_ == GizmoPart::vertical)
   {
      moveBy.x = 0.0f;
   }
   else if (gizmo_.selectedPart_ == GizmoPart::hotizontal)
   {
      moveBy.y = 0.0f;
   }

   // Editor objects selected have higher priority of movement
   // for example when animation point is selected and it's placed on top of game object
   if (currentSelectedEditorObject_ != Object::INVALID_ID)
   {
      auto& selectedEditorObject = GetEditorObjectRef(currentSelectedEditorObject_);

      selectedEditorObject.Move(camera_.ConvertToCameraVector(moveBy));
      gui_.ObjectUpdated(selectedEditorObject.GetLinkedObjectID());
   }
   else if (currentSelectedGameObject_ != Object::INVALID_ID)
   {
      auto& selectedGameObject = currentLevel_->GetGameObjectRef(currentSelectedGameObject_);

      selectedGameObject.Move(camera_.ConvertToCameraVector(moveBy));
      gui_.ObjectUpdated(currentSelectedGameObject_);
   }
   else
   {
      for (const auto object : selectedObjects_)
      {
         auto& baseObject = currentLevel_->GetObjectRef(object);
         auto& gameObject = dynamic_cast< GameObject& >(baseObject);

         gameObject.Move(camera_.ConvertToCameraVector(moveBy));
         gameObject.GetSprite().SetInitialPosition(gameObject.GetSprite().GetPosition());

         gui_.ObjectUpdated(object);

         auto* animatable = dynamic_cast< Animatable* >(&baseObject);
         if (animatable)
         {
            animatable->SetAnimationStartLocation(gameObject.GetPosition());
            UpdateAnimationData(object);
         }
      }
   }

   gizmo_.Move(camera_.ConvertToCameraVector(moveBy));
}

void
Editor::ScaleLogic(const glm::vec2& currentCursorPos)
{
   // Calculate the value of cursor movement
   // For example:
   // - cursor was moved to the right then movementVector.x is positive, negative otherwise
   // - cursor was moved to the top of window then movementVector.y is positive, negative
   // otherwise
   const auto movementVector = currentCursorPos - lastCursorPosition_;

   // Compute the bigger factor (magnitute hence abs())
   // Go with `-movementVector.x` because it feels right
   const auto movementVal =
      std::abs(movementVector.x) > std::abs(movementVector.y) ? movementVector.x : movementVector.y;

   constexpr auto maxVal = 1.0f;

   const auto value = glm::clamp(movementVal, -maxVal, maxVal);

   constexpr auto scaleFactor = 15.0f;
   auto scaleBy = glm::vec2{value, value} * scaleFactor;
   // By default we apply scaling on both axis
   if (gizmo_.selectedPart_ == GizmoPart::vertical)
   {
      scaleBy.x = 0.0f;
   }
   else if (gizmo_.selectedPart_ == GizmoPart::hotizontal)
   {
      scaleBy.y = 0.0f;
   }

   // Editor objects selected have higher priority
   // for example when animation point is selected and it's placed on top of game object
   if (currentSelectedEditorObject_ != Object::INVALID_ID)
   {
      auto& selectedEditorObject = GetEditorObjectRef(currentSelectedEditorObject_);

      selectedEditorObject.Scale(scaleBy, true);
   }
   else
   {
      for (const auto object : selectedObjects_)
      {
         auto& gameObject = dynamic_cast< GameObject& >(currentLevel_->GetObjectRef(object));

         gameObject.Scale(scaleBy, true);
      }
   }
}

void
Editor::HandleMouseDrag(const glm::vec2& currentCursorPos, const glm::vec2& axis)
{
   if (LMBPressedLastUpdate_)
   {
      if (gizmo_.mouseOnGizmo_)
      {
         ShowCursor(false);

         // Rotate camera (or currently selected Object)
         if (gizmo_.currentState_ == GizmoState::rotate)
         {
            RotateLogic(currentCursorPos);
         }
         // Move camera (or currently selected Object)
         else if (gizmo_.currentState_ == GizmoState::translate)
         {
            MoveLogic(axis);
         }
         else
         {
            ScaleLogic(currentCursorPos);
         }
      }
      else
      {
         const auto globalPos = ScreenToGlobal(currentCursorPos);

         // Initial mouse drag on selection
         if (selectStartPos_ == glm::vec2{})
         {
            selectingObjects_ = true;
            selectStartPos_ = globalPos;
         }

         const auto minVal = glm::vec2(glm::min(selectStartPos_.x, globalPos.x),
                                       glm::min(selectStartPos_.y, globalPos.y));
         const auto maxVal = glm::vec2(glm::max(selectStartPos_.x, globalPos.x),
                                       glm::max(selectStartPos_.y, globalPos.y));

         selectRect_[0] = glm::vec2{minVal.x, maxVal.y};
         selectRect_[1] = maxVal;
         selectRect_[2] = glm::vec2{maxVal.x, minVal.y};
         selectRect_[3] = minVal;
      }
   }
   else if (RMBPressedLastUpdate_)
   {
      ShowCursor(false);
      camera_.Move(glm::vec3{currentCursorPos - lastCursorPosition_, 0.0f});
   }

   mouseDrag_ = true;
}

void
Editor::SetMouseOnObject()
{
   if (mouseDrag_)
   {
      if (movementOnEditorObject_ and currentSelectedEditorObject_ != Object::INVALID_ID)
      {
         const auto& selectedEditorObject = GetEditorObjectRef(currentSelectedEditorObject_);
         InputManager::SetMousePos(selectedEditorObject.GetScreenPositionPixels());
      }
      else if (movementOnGameObject_ and currentSelectedGameObject_ != Object::INVALID_ID)
      {
         InputManager::SetMousePos(
            currentLevel_->GetGameObjectRef(currentSelectedGameObject_).GetScreenPositionPixels());
      }
   }
}

void
Editor::HandleGameObjectClicked(Object::ID newSelectedGameObject, bool groupSelect, bool fromGUI)
{
   const auto objectAlreadySelected =
      stl::find(selectedObjects_, newSelectedGameObject) != selectedObjects_.end();
   const auto mainSelectedObject = currentSelectedGameObject_ == newSelectedGameObject;

   if (mainSelectedObject and groupSelect)
   {
      UnselectGameObject(newSelectedGameObject, groupSelect);
   }
   else if (objectAlreadySelected)
   {
      if (groupSelect)
      {
         UnselectGameObject(newSelectedGameObject, groupSelect);
      }
      else
      {
         SelectGameObject(newSelectedGameObject);
      }
   }
   else
   {
      if (not groupSelect)
      {
         UnselectAll();
         SelectGameObject(newSelectedGameObject);
      }
      else
      {
         selectedObjects_.push_back(newSelectedGameObject);
      }
   }

   movementOnGameObject_ = !fromGUI;
}

void
Editor::HandleObjectSelected(Object::ID objectID, bool fromGUI)
{
   switch (Object::GetTypeFromID(objectID))
   {
      case ObjectType::ANIMATION_POINT: {
         auto it = stl::find_if(animationPoints_, [objectID](const auto& point) {
            return point.GetLinkedObjectID() == objectID;
         });

         if (it != animationPoints_.end())
         {
            HandleEditorObjectSelected(*it, fromGUI);
         }
      }
      break;

      case ObjectType::ENEMY:
      case ObjectType::OBJECT: {
         HandleGameObjectClicked(objectID, false, fromGUI);
      }
      break;

      default: {
      }
   }
}

Object::ID
Editor::GetSelectedEditorObject()
{
   Object::ID selected = Object::INVALID_ID;

   if (currentSelectedEditorObject_ != Object::INVALID_ID)
   {
      selected = GetEditorObjectRef(currentSelectedEditorObject_).GetLinkedObjectID();
   }

   return selected;
}

Object::ID
Editor::GetSelectedGameObject() const
{
   Object::ID selected = Object::INVALID_ID;

   if (currentSelectedGameObject_ != Object::INVALID_ID)
   {
      selected = currentSelectedGameObject_;
   }

   return selected;
}

const std::vector< Object::ID >&
Editor::GetSelectedObjects() const
{
   return selectedObjects_;
}

void
Editor::SelectGameObject(Object::ID newSelectedGameObject)
{
   currentSelectedGameObject_ = newSelectedGameObject;
   gui_.ObjectSelected(currentSelectedGameObject_);

   // Make sure to render animation points if needed
   auto& gameObject = currentLevel_->GetGameObjectRef(newSelectedGameObject);
   const auto* animatable = dynamic_cast< Animatable* >(&gameObject);

   if (animatable and animatable->GetRenderAnimationSteps())
   {
      SetRenderAnimationPoints(true);
   }

   gizmo_.Update(gameObject.GetCenteredPosition(), gameObject.GetSprite().GetRotation());
   gizmo_.Show();
   gizmoActive_ = true;
}

void
Editor::SetVisibleAnimationPoints(const Animatable& animatable, bool visible)
{
   const auto& animationPoints = animatable.GetAnimationKeypoints();

   for (const auto& animationPoint : animationPoints)
   {
      auto it = std::ranges::find_if(animationPoints_, [&animationPoint](auto& editorObject) {
         return editorObject.GetLinkedObjectID() == animationPoint.GetID();
      });

      if (it != animationPoints_.end())
      {
         it->SetVisible(visible);
      }
   }
}

void
Editor::RecalculateGizmoPos()
{
   if (selectedObjects_.empty())
   {
      return;
   }

   auto& firstObject = currentLevel_->GetGameObjectRef(selectedObjects_.front());

   auto gizmoPos = firstObject.GetCenteredPosition();
   glm::vec2 min = gizmoPos;
   glm::vec2 max = gizmoPos;

   for (const auto object : selectedObjects_)
   {
      const auto& objectPos = currentLevel_->GetGameObjectRef(object).GetCenteredPosition();
      min = glm::vec2{
         glm::min(min.x, objectPos.x),
         glm::min(min.y, objectPos.y),
      };

      max = glm::vec2{
         glm::max(max.x, objectPos.x),
         glm::max(max.y, objectPos.y),
      };
   }

   gizmo_.Update((min + max) / 2.0f,
                 selectedObjects_.size() == 1 ? firstObject.GetSprite().GetRotation() : 0.0f);
}

void
Editor::UnselectGameObject(Object::ID object, bool groupSelect)
{
   movementOnGameObject_ = false;

   gui_.ObjectUnselected(object);

   const auto* animatable = dynamic_cast< Animatable* >(&currentLevel_->GetObjectRef(object));

   if (animatable and animatable->GetRenderAnimationSteps())
   {
      SetVisibleAnimationPoints(*animatable, false);
   }

   if (currentSelectedGameObject_ == object)
   {
      currentSelectedGameObject_ = Object::INVALID_ID;
   }
   else if (groupSelect)
   {
      auto it = stl::find(selectedObjects_, object);
      if (it != selectedObjects_.end())
      {
         selectedObjects_.erase(it);
      }
   }


   if (selectedObjects_.empty())
   {
      gizmo_.Hide();
      gizmoActive_ = false;
   }
   else if (currentSelectedGameObject_ == Object::INVALID_ID)
   {
      RecalculateGizmoPos();
   }
}

void
Editor::UnselectAll()
{
   if (currentSelectedEditorObject_ != Object::INVALID_ID)
   {
      UnselectEditorObject(currentSelectedEditorObject_);
   }

   for (const auto object : selectedObjects_)
   {
      gui_.ObjectUnselected(object);
   }
   selectedObjects_.clear();
}

void
Editor::HandleEditorObjectSelected(EditorObject& newSelectedEditorObject, bool fromGUI)
{
   if (editorObjectSelected_ && (newSelectedEditorObject.GetID() != currentSelectedEditorObject_))
   {
      UnselectEditorObject(currentSelectedEditorObject_);
   }

   currentSelectedEditorObject_ = newSelectedEditorObject.GetID();
   editorObjectSelected_ = true;
   movementOnEditorObject_ = !fromGUI;

   newSelectedEditorObject.SetObjectSelected();

   gizmo_.Show();
   gizmoActive_ = true;
}

EditorObject&
Editor::GetEditorObjectRef(Object::ID object)
{
   auto animationPointIt = stl::find_if(animationPoints_, [object](const auto& editorObject) {
      return editorObject.GetID() == object;
   });

   utils::Assert(
      animationPointIt != animationPoints_.end(),
      fmt::format("GetEditorObjectRef didn't find any EditorObject for given ID. Type of Object is "
                  "{} with ID = {}\n",
                  Object::GetTypeString(object), object));

   return *animationPointIt;
}

void
Editor::UnselectEditorObject(Object::ID object)
{
   editorObjectSelected_ = false;
   movementOnEditorObject_ = false;

   auto& editorObject = GetEditorObjectRef(object);
   editorObject.SetObjectUnselected();
   currentSelectedEditorObject_ = Object::INVALID_ID;

   gizmo_.Hide();
   gizmoActive_ = false;
}

void
Editor::CheckIfObjectGotSelected(const glm::vec2& cursorPosition, bool groupSelect)
{
   auto newSelectedEditorObject = stl::find_if(animationPoints_, [cursorPosition](auto& object) {
      return object.IsVisible() && object.CheckIfCollidedScreenPosion(cursorPosition);
   });

   if (newSelectedEditorObject != animationPoints_.end())
   {
      HandleEditorObjectSelected(*newSelectedEditorObject);
   }
   else
   {
      auto newSelectedObject =
         currentLevel_->GetGameObjectOnLocationAndLayer(cursorPosition, renderLayerToDraw_);

      if (newSelectedObject != Object::INVALID_ID)
      {
         HandleGameObjectClicked(newSelectedObject, groupSelect, false);
      }
   }
}

std::vector< Object::ID >
Editor::GetObjectsInArea(const std::array< glm::vec2, 4 >& area) const
{
   std::set< Object::ID > objectsList = {};

   if (glm::length(area.at(1) - area.at(3)) < currentLevel_->GetTileSize() / 2)
   {
      return {};
   }

   const auto tiles = currentLevel_->GetTilesFromRectangle(area);
   auto& pathfinder = currentLevel_->GetPathfinder();

   for (const auto& tile : tiles)
   {
      const auto& objectsOnNode = pathfinder.GetNodeFromTile(tile).objectsOnThisNode_;

      if (renderLayerToDraw_ != -1)
      {
         for (const auto& object : objectsOnNode)
         {
            const auto& gameObject = currentLevel_->GetGameObjectRef(object);
            if (gameObject.GetSprite().GetRenderInfo().layer == renderLayerToDraw_)
            {
               objectsList.insert(object);
            }
         }
      }
      else
      {
         for (const auto object : objectsOnNode)
         {
            objectsList.insert(object);
         }
      }
   }

   // Select animation points if we have selected game object
   if (currentSelectedGameObject_ != Object::INVALID_ID)
   {
   }

   return {objectsList.begin(), objectsList.end()};
}

int32_t
Editor::GetRenderLayerToDraw() const
{
   return renderLayerToDraw_;
}

void
Editor::SetRenderLayerToDraw(int32_t layer)
{
   renderLayerToDraw_ = layer;
}

void
Editor::SetupRendererData() const
{
   renderer::RecreateQuadPipeline();

   DrawGrid();
   renderer::CreateLinePipeline();
   renderer::SetupLineData();
   renderer::UpdateLineData();
}


void
Editor::AddAnimationPoint(const glm::vec2& position)
{
   GetCamera().SetCameraAtPosition(position);
   AddObject(ObjectType::ANIMATION_POINT);
   SetRenderAnimationPoints(true);
}

void
Editor::SelectAnimationPoint(const AnimationPoint& node)
{
   GetCamera().SetCameraAtPosition(node.m_end);
   HandleObjectSelected(node.GetID(), true);
   SetRenderAnimationPoints(true);
}

void
Editor::AddToWorkQueue(const WorkQueue::WorkUnit& work, const WorkQueue::Precondition& prec)
{
   workQueue_.PushWorkUnit(prec, work);
}

bool
Editor::IsAnyObjectSelected() const
{
   return editorObjectSelected_ or currentSelectedEditorObject_ != Object::INVALID_ID
          or currentSelectedGameObject_ != Object::INVALID_ID or not selectedObjects_.empty();
}

void
Editor::ActionOnObject(Editor::ACTION action, Object::ID object)
{
   switch (action)
   {
      case ACTION::UNSELECT:

         if (Object::GetTypeFromID(object) == ObjectType::EDITOR_OBJECT)
         {
            UnselectEditorObject(object);
         }
         else
         {
            UnselectGameObject(object, true);
         }
         break;

      case ACTION::REMOVE:

         if (Object::GetTypeFromID(object) == ObjectType::EDITOR_OBJECT)
         {
            UnselectEditorObject(object);
            auto& currentySelectedObj = GetEditorObjectRef(currentSelectedEditorObject_);
            gui_.ObjectDeleted(currentySelectedObj.GetLinkedObjectID());
            currentySelectedObj.DeleteLinkedObject();

            if (Object::GetTypeFromID(currentySelectedObj.GetLinkedObjectID())
                == ObjectType::ANIMATION_POINT)
            {
               animationPoints_.erase(
                  stl::find_if(animationPoints_, [this](const auto& animationPoint) {
                     return animationPoint.GetID() == currentSelectedEditorObject_;
                  }));
            }
         }
         else
         {
            UnselectGameObject(object, true);
            gui_.ObjectDeleted(object);

            currentLevel_->DeleteObject(object);
         }
         break;
      default: {
      }
   }
}

void
Editor::Render(VkCommandBuffer cmdBuffer)
{
   if (levelLoaded_)
   {
      auto& renderData = renderer::Data::renderData_[renderer::GetCurrentlyBoundType()];

      currentLevel_->GetSprite().Render();

      gizmo_.Render();
      DrawAnimationPoints();

      currentLevel_->RenderGameObjects();

      vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.pipeline);

      auto offsets = std::to_array< const VkDeviceSize >({0});
      vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.pipelineLayout,
                              0, 1, &renderData.descriptorSets[renderer::Data::currentFrame_], 0,
                              nullptr);
      renderer::QuadShader::PushConstants pushConstants = {};
      pushConstants.selectedIdx = -1.0f;

      if (currentSelectedGameObject_ != Object::INVALID_ID)
      {
         const auto tmpIdx =
            currentLevel_->GetGameObjectRef(currentSelectedGameObject_).GetSprite().GetRenderIdx();
         pushConstants.selectedIdx = static_cast< float >(tmpIdx);
      }

      vkCmdPushConstants(cmdBuffer, renderData.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                         sizeof(renderer::QuadShader::PushConstants), &pushConstants);

      for (int32_t layer = renderer::NUM_LAYERS - 1; layer >= 0; --layer)
      {
         const auto idx = static_cast< size_t >(layer);
         const auto& numObjects = renderData.numMeshes.at(idx);
         const auto renderThisLayer = renderLayerToDraw_ == -1 ? true : renderLayerToDraw_ == layer;

         if (numObjects == 0 or !renderThisLayer)
         {
            continue;
         }

         vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &renderData.vertexBuffer.at(idx).buffer_,
                                offsets.data());

         vkCmdBindIndexBuffer(cmdBuffer, renderData.indexBuffer.at(idx).buffer_, 0,
                              VK_INDEX_TYPE_UINT32);

         vkCmdDrawIndexed(cmdBuffer, numObjects * renderer::INDICES_PER_SPRITE, 1, 0, 0, 0);
      }

      // DRAW LINES
      vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                        renderer::EditorData::linePipeline_);

      vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &renderer::EditorData::lineVertexBuffer.buffer_,
                             offsets.data());

      vkCmdBindIndexBuffer(cmdBuffer, renderer::EditorData::lineIndexBuffer.buffer_, 0,
                           VK_INDEX_TYPE_UINT32);

      vkCmdBindDescriptorSets(
         cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer::EditorData::linePipelineLayout_, 0,
         1, &renderer::EditorData::lineDescriptorSets_[renderer::Data::currentFrame_], 0, nullptr);

      renderer::LineShader::PushConstants linePushConstants = {};
      linePushConstants.color = glm::vec4(0.4f, 0.5f, 0.6f, static_cast< float >(drawGrid_));

      vkCmdPushConstants(cmdBuffer, renderer::EditorData::linePipelineLayout_,
                         VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                         sizeof(renderer::LineShader::PushConstants), &linePushConstants);

      vkCmdDrawIndexed(cmdBuffer, renderer::EditorData::numGridLines * renderer::INDICES_PER_LINE,
                       1, 0, 0, 0);

      linePushConstants.color = glm::vec4(0.5f, 0.0f, 0.0f, 1.0f);
      vkCmdPushConstants(cmdBuffer, renderer::EditorData::linePipelineLayout_,
                         VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                         sizeof(renderer::LineShader::PushConstants), &linePushConstants);
      vkCmdDrawIndexed(cmdBuffer, renderer::EditorData::curDynLineIdx, 1,
                       renderer::EditorData::numGridLines * renderer::INDICES_PER_LINE, 0, 0);
   }

   gui_.Render(cmdBuffer);
}

void
Editor::DrawAnimationPoints()
{
   bool drawLines = false;
   std::vector< Object::ID > animaltionPointIDs = {};
   glm::vec2 lineStart = {};

   if (currentSelectedGameObject_ != Object::INVALID_ID)
   {
      auto& baseObject = currentLevel_->GetObjectRef(currentSelectedGameObject_);
      auto* animatable = dynamic_cast< Animatable* >(&baseObject);

      if (animatable and animatable->GetRenderAnimationSteps())
      {
         drawLines = true;

         const auto& animationPoints = animatable->GetAnimationKeypoints();
         std::ranges::transform(
            animationPoints, std::back_inserter(animaltionPointIDs),
            [](const auto& animationKeyPoint) { return animationKeyPoint.GetID(); });

         lineStart = animatable->GetAnimationStartLocation();
      }
   }

   for (auto& point : animationPoints_)
   {
      point.Render();

      if (drawLines)
      {
         if (std::ranges::find(animaltionPointIDs, point.GetLinkedObjectID())
             != animaltionPointIDs.end())
         {
            renderer::DrawDynamicLine(lineStart, point.GetPosition());
            lineStart = point.GetCenteredPosition();
         }
      }
   }

   renderer::UpdateLineData(renderer::EditorData::numGridLines);
}

void
Editor::DrawBoundingBoxes()
{
   auto drawBoundingBox = [](const renderer::Sprite& sprite) {
      const auto rect = sprite.GetTransformedRectangle();
      renderer::DrawDynamicLine(rect[0], rect[1]);
      renderer::DrawDynamicLine(rect[1], rect[2]);
      renderer::DrawDynamicLine(rect[2], rect[3]);
      renderer::DrawDynamicLine(rect[3], rect[0]);
   };

   for (const auto object : selectedObjects_)
   {
      drawBoundingBox(currentLevel_->GetGameObjectRef(object).GetSprite());
   }

   if (currentSelectedGameObject_ != Object::INVALID_ID)
   {
      drawBoundingBox(currentLevel_->GetGameObjectRef(currentSelectedGameObject_).GetSprite());
   }

   if (currentSelectedEditorObject_ != Object::INVALID_ID)
   {
      drawBoundingBox(GetEditorObjectRef(currentSelectedEditorObject_).GetSprite());
   }
}

void
Editor::DrawGrid() const
{
   const auto levelSize = currentLevel_->GetSize();
   const auto grad = gridCellSize_;

   const auto w = levelSize.x / grad;
   const auto h = levelSize.y / grad;
   // const auto offset = glm::ivec2(0, grad);

   for (int i = 0; i <= h; ++i)
   {
      renderer::DrawLine(glm::vec2(0, i * grad), glm::vec2(levelSize.x, i * grad));
   }

   for (int i = 0; i <= w; ++i)
   {
      renderer::DrawLine(glm::vec2(i * grad, 0), glm::vec2(i * grad, levelSize.y));
   }

   renderer::EditorData::numGridLines = renderer::EditorData::numLines;
}

void
Editor::SetGridData(bool render, int32_t cellSize)
{
   drawGrid_ = render;
   gridCellSize_ = cellSize;
}

std::pair< bool, int32_t >
Editor::GetGridData() const
{
   return {drawGrid_, gridCellSize_};
}

time::TimeStep
Editor::GetFrameTime() const
{
   return timeLastFrame_;
}

time::TimeStep
Editor::GetUpdateUITime() const
{
   return uiTime_;
}

time::TimeStep
Editor::GetRenderTime() const
{
   return renderTime_;
}

std::pair< uint32_t, uint32_t >
Editor::GetRenderOffsets() const
{
   return {numObjects_, numPathfinderNodes_};
}

void
Editor::FreeLevelData()
{
   if (levelLoaded_)
   {
      if (currentSelectedEditorObject_)
      {
         UnselectEditorObject(currentSelectedEditorObject_);
      }
      if (currentSelectedGameObject_ != Object::INVALID_ID)
      {
         UnselectGameObject(currentSelectedGameObject_, false);
      }

      currentLevel_.reset();
      pathfinderNodes_.clear();
      animationPoints_.clear();

      renderer::FreeData(renderer::ApplicationType::EDITOR, false);
   }
}

void
Editor::CreateLevel(const std::string& name, const glm::ivec2& size)
{
   FreeLevelData();

   currentLevel_ = std::make_shared< Level >();
   currentLevel_->Create(this, name, size);

   camera_.Create(glm::vec3(0.0f, 0.0f, 0.0f), window_.GetSize());
   camera_.SetLevelSize(currentLevel_->GetSize());

   levelLoaded_ = true;
   levelFileName_ = (LEVELS_DIR / (name + ".dgl")).string();
   gui_.LevelLoaded(currentLevel_);

   currentLevel_->GenerateTextureForCollision();

   SetupRendererData();
}

void
Editor::LoadLevel(const std::string& levelPath)
{
   FreeLevelData();

   renderer::SetAppMarker(renderer::ApplicationType::EDITOR);

   {
      SCOPED_TIMER("Total level load");

      levelFileName_ = levelPath;

      // Should we actually compute total num points?
      animationPoints_.reserve(1000);

      currentLevel_ = std::make_shared< Level >();
      currentLevel_->Load(this, levelPath);

      {
         SCOPED_TIMER("Animation points setup");

         const auto& enemies = currentLevel_->GetEnemies();
         for (const auto& enemy : enemies)
         {
            const auto& animationPoints = enemy.GetAnimationKeypoints();

            for (const auto& point : animationPoints)
            {
               auto& editorObject = animationPoints_.emplace_back(
                  this, point.m_end, glm::vec2(20, 20), "NodeSprite.png", point.GetID());

               editorObject.SetName(fmt::format("AnimationPoint{}", enemy.GetName()));
               editorObject.SetVisible(false);
               editorObject.Render();
            }
         }
      }

      camera_.Create(glm::vec3(currentLevel_->GetPlayer().GetPosition(), 0.0f), window_.GetSize());
      camera_.SetLevelSize(currentLevel_->GetSize());

      levelLoaded_ = true;

      gui_.LevelLoaded(currentLevel_);

      currentLevel_->GenerateTextureForCollision();

      window_.MakeFocus();

      gizmo_.Initialize();
   }

   SetupRendererData();
}

void
Editor::SaveLevel(const std::string& levelPath)
{
   levelFileName_ = levelPath;
   currentLevel_->Save(levelFileName_);
}

void
Editor::AddGameObject(ObjectType objectType, const glm::vec2& position)
{
   HandleGameObjectClicked(currentLevel_->AddGameObject(objectType, position), false, false);

   shouldUpdateRenderer_ = true;
}

void
Editor::CopyGameObjects(const std::vector< Object::ID >& objectsToCopy)
{
   std::vector< Object::ID > newObjects = {};

   const auto mousePos = ScreenToGlobal(InputManager::GetMousePos());
   auto offset = glm::vec2{};
   for (const auto objectToCopy : objectsToCopy)
   {
      const auto& object = currentLevel_->GetGameObjectRef(objectToCopy);
      const auto& position = object.GetPosition();
      const auto& size = object.GetSize();
      const auto hasCollision = object.GetHasCollision();
      const auto& textureName = object.GetSprite().GetTextureName();
      const auto rotation = object.GetSprite().GetRotation();

      auto newObjectID =
         currentLevel_->AddGameObject(Object::GetTypeFromID(objectToCopy), position + offset);

      // Set it only once
      if (offset == glm::vec2{})
      {
         offset = (mousePos - position);
      }

      auto& newObject = currentLevel_->GetGameObjectRef(newObjectID);
      newObject.SetSize(size);
      newObject.SetHasCollision(hasCollision);
      newObject.GetSprite().SetTextureFromFile(textureName);
      newObject.Rotate(rotation);

      gui_.ObjectUpdated(newObjectID);
      newObjects.push_back(newObjectID);

      shouldUpdateRenderer_ = true;
   }
}

void
Editor::AddObject(ObjectType objectType)
{
   utils::Assert(objectType == ObjectType::ANIMATION_POINT,
                 "Editor::AddObject only supports Animation Points!\n");

   if (currentSelectedGameObject_ == Object::INVALID_ID)
   {
      Logger::Warn("Added new Animation point without currently selected object!");
   }

   auto& baseObject = currentLevel_->GetObjectRef(currentSelectedGameObject_);
   auto& animatable = dynamic_cast< Animatable& >(baseObject);
   auto& gameObject = dynamic_cast< GameObject& >(baseObject);
   auto newNode = animatable.CreateAnimationNode(
      currentSelectedGameObject_,
      gameObject.GetPosition() + static_cast< glm::vec2 >(gameObject.GetSize()));

   auto& newObject = animationPoints_.emplace_back(this, newNode.m_end, glm::ivec2(20, 20),
                                                   "NodeSprite.png", newNode.GetID());
   animatable.ResetAnimation();

   shouldUpdateRenderer_ = true;

   HandleEditorObjectSelected(newObject);
}

void
Editor::ToggleAnimateObject()
{
   if (animateGameObject_)
   {
      // TODO: This should be changed in future!
      auto& enemy = dynamic_cast< Enemy& >(currentLevel_->GetObjectRef(currentSelectedGameObject_));
      enemy.SetPosition(enemy.GetInitialPosition());
      animateGameObject_ = false;
   }
   else
   {
      animateGameObject_ = true;
   }
}

bool
Editor::IsObjectAnimated() const
{
   return animateGameObject_;
}

void
Editor::PlayLevel()
{
   // TODO: For future we'd want to check if anything got changed,
   //       so we don't always save (this can get costly later!)
   currentLevel_->Save(levelFileName_);
   playGame_ = true;
}

void
Editor::LaunchGameLoop()
{
   {
      Game game;
      game.Init("GameInit.json", false);
      game.LoadLevel(levelFileName_);

      // TODO: Create game-thread and run it inside
      game.MainLoop();
   }

   renderer::FreeData(renderer::ApplicationType::GAME, true);
   playGame_ = false;

   renderer::SetAppMarker(renderer::ApplicationType::EDITOR);
}

void
Editor::RenderNodes(bool render)
{
   renderPathfinderNodes_ = render;
   currentLevel_->RenderPathfinder(render);
}

bool
Editor::GetRenderNodes() const
{
   return renderPathfinderNodes_;
}

void
Editor::SetRenderAnimationPoints(bool render)
{
   auto& animatable =
      dynamic_cast< Animatable& >(currentLevel_->GetObjectRef(currentSelectedGameObject_));

   if (animatable.GetRenderAnimationSteps() != render)
   {
      animatable.RenderAnimationSteps(render);
      SetVisibleAnimationPoints(animatable, render);
   }
}

void
Editor::SetLockAnimationPoints(bool lock)
{
   dynamic_cast< Animatable& >(currentLevel_->GetObjectRef(currentSelectedGameObject_))
      .LockAnimationSteps(lock);
}

void
Editor::Update()
{
   if (animateGameObject_ && currentSelectedGameObject_ != Object::INVALID_ID)
   {
      auto& objectBase = currentLevel_->GetObjectRef(currentSelectedGameObject_);
      auto& animatable = dynamic_cast< Animatable& >(objectBase);
      auto& gameObject = dynamic_cast< GameObject& >(objectBase);
      auto moveBy = animatable.SingleAnimate(deltaTime_);

      if (glm::length(moveBy) > 0.0f)
      {
         const auto prevPosition = gameObject.GetPreviousPosition();
         const auto direction = gameObject.GetPosition() - prevPosition;

         const auto viewAngle = glm::atan(direction.y, direction.x);

         gameObject.Rotate(viewAngle);
         gameObject.Move(moveBy);
      }
      else if (animatable.AnimationFinished())
      {
         animateGameObject_ = false;
      }
   }

   DrawBoundingBoxes();

   if (mouseDrag_ and selectRect_ != std::array< glm::vec2, 4 >{})
   {
      renderer::DrawDynamicLine(selectRect_[0], selectRect_[1]);
      renderer::DrawDynamicLine(selectRect_[1], selectRect_[2]);
      renderer::DrawDynamicLine(selectRect_[2], selectRect_[3]);
      renderer::DrawDynamicLine(selectRect_[3], selectRect_[0]);
   }

   {
      const time::ScopedTimer uiTImer(&uiTime_);
      gui_.UpdateUI();
   }

   auto& renderData = renderer::GetRenderData();
   renderData.viewMat = camera_.GetViewMatrix();
   renderData.projMat = camera_.GetProjectionMatrix();

   renderer::UpdateData();
}

void
Editor::UpdateAnimationData(Object::ID object)
{
   dynamic_cast< Animatable& >(currentLevel_->GetObjectRef(object)).UpdateAnimationData();
}

glm::vec2
Editor::GetWindowSize() const
{
   return window_.GetSize();
}

const glm::mat4&
Editor::GetProjection() const
{
   return camera_.GetProjectionMatrix();
}

const glm::mat4&
Editor::GetViewMatrix() const
{
   return camera_.GetViewMatrix();
}

float
Editor::GetZoomLevel() const
{
   return camera_.GetZoomLevel();
}

bool
Editor::IsRunning() const
{
   return isRunning_;
}

void
Editor::Shutdown()
{
   isRunning_ = false;
}

void
Editor::MainLoop()
{
   auto singleFrameTimer = time::microseconds(0);
   time::Stopwatch watch;
   while (IsRunning())
   {
      watch.Start();
      timer_.ToggleTimer();
      singleFrameTimer += timer_.GetMicroDeltaTime();

      while (IsRunning() and (singleFrameTimer.count() >= TARGET_TIME_MICRO))
      {
         const time::ScopedTimer frameTimer(&timeLastFrame_);

         deltaTime_ = timer_.GetMsDeltaTime();
         InputManager::PollEvents();

         if (windowInFocus_)
         {
            // Run all deffered work units
            workQueue_.RunWorkUnits();

            Update();

            const time::ScopedTimer renderTimer(&renderTime_);
            renderer::Render(this);
         }

         timeLastFrame_ = watch.Stop();

         if (frameTimer_ > 1.0f)
         {
            framesLastSecond_ = frames_;
            frameTimer_ = 0.0f;
            frames_ = 0;
         }

         ++frames_;
         frameTimer_ += TARGET_TIME_S;

         renderer::EditorData::curDynLineIdx = 0;
         if (levelLoaded_ and currentLevel_->GetPathfinder().IsInitialized())
         {
            currentLevel_->UpdateCollisionTexture();
            currentLevel_->GetPathfinder().ClearPerFrameData();
         }

         if (playGame_)
         {
            LaunchGameLoop();
         }

         timer_.ToggleTimer();
      }
   }
}

} // namespace looper
