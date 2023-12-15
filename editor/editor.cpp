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
   m_window = std::make_unique< renderer::Window >(screenSize, "Editor", true);

   InputManager::Init(m_window->GetWindowHandle());

   renderer::VulkanRenderer::Initialize(m_window->GetWindowHandle(),
                                        renderer::ApplicationType::EDITOR);
   gui_.Init();
   InputManager::RegisterForInput(m_window->GetWindowHandle(), this);
}

void
Editor::ShowCursor(bool choice)
{
   m_window->ShowCursor(choice);
}

void
Editor::HandleCamera()
{
   auto cameraMoveBy = glm::vec2();

   if (!EditorGUI::IsBlockingEvents() && m_levelLoaded)
   {
      if (InputManager::CheckKeyPressed(GLFW_KEY_W))
      {
         cameraMoveBy += glm::vec2(0.0f, -1.0f);
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_S))
      {
         cameraMoveBy += glm::vec2(0.0f, 1.0f);
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_A))
      {
         cameraMoveBy += glm::vec2(-1.0f, 0.0f);
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_D))
      {
         cameraMoveBy += glm::vec2(1.0f, 0);
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_SPACE))
      {
         m_camera.SetCameraAtPosition({0.0f, 0.0f, 0.0f});
      }

      m_camera.Move(glm::vec3(cameraMoveBy, 0.0f));
   }
}

void
Editor::KeyCallback(KeyEvent& event)
{
   if (event.action_ == GLFW_PRESS)
   {
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
            if (currentEditorObjectSelected_)
            {
               ActionOnObject(action, currentEditorObjectSelected_->GetID());
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
   if (!playGame_ && !EditorGUI::IsBlockingEvents() && m_levelLoaded)
   {
      m_camera.Zoom(static_cast< float >(event.xOffset_ + event.yOffset_));
      event.handled_ = true;
   }
}

void
Editor::MouseButtonCallback(MouseButtonEvent& event)
{
   if (!playGame_ && !EditorGUI::IsBlockingEvents() && m_levelLoaded)
   {
      const auto mousePressed = event.action_ == GLFW_PRESS;
      LMBPressedLastUpdate_ = mousePressed and event.button_ == GLFW_MOUSE_BUTTON_1;
      RMBPressedLastUpdate_ = mousePressed and event.button_ == GLFW_MOUSE_BUTTON_2;

      if (mousePressed)
      {
         CheckIfObjectGotSelected(InputManager::GetMousePos(),
                                  InputManager::CheckKeyPressed(GLFW_KEY_LEFT_CONTROL));
      }
      else if (movementOnEditorObject_ or movementOnGameObject_ or selectingObjects_)
      {
         // Object movement finished
         ShowCursor(true);
         SetMouseOnObject();

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

               for (const auto object : selectedObjects)
               {
                  gui_.ObjectSelected(object);
               }

               for (const auto& object : selectedObjects_)
               {
                  gui_.ObjectUnselected(object);
               }

               selectedObjects_ = selectedObjects;
            }
            selectStartPos_ = glm::vec2{};
            selectRect_ = std::array< glm::vec2, 4 >{};
         }

         mouseDrag_ = false;
         selectingObjects_ = false;
      }

      event.handled_ = true;
   }
}

void
Editor::CursorPositionCallback(CursorPositionEvent& event)
{
   if (!playGame_ && !EditorGUI::IsBlockingEvents() && m_levelLoaded)
   {
      const auto currentCursorPosition = glm::vec2(event.xPos_, event.yPos_);

      if (RMBPressedLastUpdate_ or LMBPressedLastUpdate_)
      {
         HandleMouseDrag(currentCursorPosition, currentCursorPosition - lastCursorPosition_);
      }
      else
      {
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

   if (movementOnEditorObject_ || movementOnGameObject_)
   {
      // Editor objects selected have higher priority of movement
      // for example when animation point is selected and it's placed on top of game object
      if (movementOnEditorObject_)
      {
         currentEditorObjectSelected_->Rotate(-angle, true);
         gui_.ObjectUpdated(currentEditorObjectSelected_->GetLinkedObjectID());
      }
      else
      {
         dynamic_cast< GameObject& >(m_currentLevel->GetObjectRef(currentSelectedGameObject_))
            .Rotate(-angle, true);
         gui_.ObjectUpdated(currentSelectedGameObject_);
      }
   }
   else
   {
      m_camera.Rotate(angle);
   }
}

void
Editor::MoveLogic(const glm::vec2& axis)
{
   const auto moveBy = glm::vec3(axis, 0.0f);

   if (movementOnEditorObject_ || movementOnGameObject_)
   {
      // Editor objects selected have higher priority of movement
      // for example when animation point is selected and it's placed on top of game object
      if (movementOnEditorObject_)
      {
         currentEditorObjectSelected_->Move(m_camera.ConvertToCameraVector(moveBy));
         gui_.ObjectUpdated(currentEditorObjectSelected_->GetLinkedObjectID());
      }
      else
      {
         for (const auto object : selectedObjects_)
         {
            auto& baseObject = m_currentLevel->GetObjectRef(object);
            auto& gameObject = dynamic_cast< GameObject& >(baseObject);

            gameObject.Move(m_camera.ConvertToCameraVector(moveBy));
            gameObject.GetSprite().SetInitialPosition(gameObject.GetSprite().GetPosition());

            gui_.ObjectUpdated(object);

            auto* animatable = dynamic_cast< Animatable* >(&baseObject);
            if (animatable)
            {
               animatable->SetAnimationStartLocation(gameObject.GetPosition());
               UpdateAnimationData();
            }
         }
      }
   }
   else
   {
      m_camera.Move(moveBy);
   }
}

void
Editor::HandleMouseDrag(const glm::vec2& currentCursorPos, const glm::vec2& axis)
{
   if (LMBPressedLastUpdate_)
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
   else if (RMBPressedLastUpdate_)
   {
      ShowCursor(false);

      // Rotate camera (or currently selected Object)
      if (InputManager::CheckKeyPressed(GLFW_KEY_LEFT_SHIFT))
      {
         RotateLogic(currentCursorPos);
      }
      // Move camera (or currently selected Object)
      else
      {
         MoveLogic(axis);
      }
   }
   else
   {
   }

   mouseDrag_ = true;
}

void
Editor::SetMouseOnObject() const
{
   if (mouseDrag_ && (movementOnEditorObject_ || movementOnGameObject_))
   {
      if (movementOnEditorObject_)
      {
         InputManager::SetMousePos(currentEditorObjectSelected_->GetScreenPositionPixels());
      }
      else if (movementOnGameObject_)
      {
         InputManager::SetMousePos(
            dynamic_cast< GameObject& >(m_currentLevel->GetObjectRef(currentSelectedGameObject_))
               .GetScreenPositionPixels());
      }
   }
}

void
Editor::HandleGameObjectSelected(Object::ID newSelectedGameObject, bool groupSelect, bool fromGUI)
{
   const auto objectAlreadySelected =
      stl::find(selectedObjects_, newSelectedGameObject) != selectedObjects_.end();
   const auto mainSelectedObject = currentSelectedGameObject_ == newSelectedGameObject;

   if (objectAlreadySelected and groupSelect and not fromGUI)
   {
      UnselectGameObject(newSelectedGameObject, groupSelect);
   }
   else if (not objectAlreadySelected or not mainSelectedObject)
   {
      if (not mainSelectedObject)
      {
         if (not groupSelect and currentSelectedGameObject_ != Object::INVALID_ID)
         {
            // unselect previously selected object
            UnselectGameObject(currentSelectedGameObject_, groupSelect);
         }
      }

      if (not objectAlreadySelected)
      {
         if (not groupSelect)
         {
            for (const auto& object : selectedObjects_)
            {
               gui_.ObjectUnselected(object);
            }
            selectedObjects_.clear();
         }

         selectedObjects_.push_back(newSelectedGameObject);
      }

      currentSelectedGameObject_ = newSelectedGameObject;


      // Make sure to render animation points if needed
      const auto* animatable =
         dynamic_cast< Animatable* >(&m_currentLevel->GetObjectRef(currentSelectedGameObject_));


      if (animatable and animatable->GetRenderAnimationSteps())
      {
         SetRenderAnimationPoints(true);
      }

      // mark new object as selected
      SelectGameObject();
      gameObjectSelected_ = true;

      if (editorObjectSelected_)
      {
         UnselectEditorObject(currentEditorObjectSelected_->GetID());
      }

      gui_.ObjectSelected(currentSelectedGameObject_);
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
            return point->GetLinkedObjectID() == objectID;
         });

         if (it != animationPoints_.end())
         {
            HandleEditorObjectSelected(*it, fromGUI);
         }
      }
      break;

      case ObjectType::ENEMY:
      case ObjectType::OBJECT: {
         HandleGameObjectSelected(objectID, false, fromGUI);
      }
      break;

      default: {
      }
   }
}

Object::ID
Editor::GetSelectedEditorObject() const
{
   Object::ID selected = Object::INVALID_ID;

   if (currentEditorObjectSelected_)
   {
      selected = currentEditorObjectSelected_->GetLinkedObjectID();
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
Editor::SelectGameObject()
{
   // currentSelectedGameObject_->SetColor({1.0f, 1.0f, 1.0f, 0.0f});
}

void
Editor::SetVisibleAnimationPoints(const Animatable& animatable, bool visible)
{
   const auto& animationPoints = animatable.GetAnimationKeypoints();

   for (const auto& animationPoint : animationPoints)
   {
      auto it = std::ranges::find_if(animationPoints_, [&animationPoint](auto& editorObject) {
         return editorObject->GetLinkedObjectID() == animationPoint.GetID();
      });

      if (it != animationPoints_.end())
      {
         (*it)->SetVisible(visible);
      }
   }
}

void
Editor::UnselectGameObject(Object::ID object, bool groupSelect)
{
   gameObjectSelected_ = false;
   movementOnGameObject_ = false;

   gui_.ObjectUnselected(object);

   const auto* animatable = dynamic_cast< Animatable* >(&m_currentLevel->GetObjectRef(object));

   if (animatable and animatable->GetRenderAnimationSteps())
   {
      SetVisibleAnimationPoints(*animatable, false);
   }

   if (currentSelectedGameObject_ == object)
   {
      currentSelectedGameObject_ = Object::INVALID_ID;
   }

   if (groupSelect)
   {
      auto it = stl::find(selectedObjects_, object);
      if (it != selectedObjects_.end())
      {
         selectedObjects_.erase(it);
      }
   }
}

void
Editor::HandleEditorObjectSelected(const std::shared_ptr< EditorObject >& newSelectedEditorObject,
                                   bool fromGUI)
{
   if (editorObjectSelected_ && (newSelectedEditorObject != currentEditorObjectSelected_))
   {
      UnselectEditorObject(currentEditorObjectSelected_->GetID());
   }

   currentEditorObjectSelected_ = newSelectedEditorObject;
   editorObjectSelected_ = true;
   movementOnEditorObject_ = !fromGUI;

   currentEditorObjectSelected_->SetObjectSelected();
}

EditorObject&
Editor::GetEditorObjectRef(Object::ID object)
{
   auto it = stl::find_if(animationPoints_, [object](const auto& editorObject) {
      return editorObject->GetID() == object;
   });

   utils::Assert(it != animationPoints_.end(),
                 fmt::format("Requested editor object = {} not found!\n", object));

   return *(*it);
}

void
Editor::UnselectEditorObject(Object::ID object)
{
   editorObjectSelected_ = false;
   movementOnEditorObject_ = false;

   auto& editorObject = GetEditorObjectRef(object);
   editorObject.SetObjectUnselected();
   currentEditorObjectSelected_.reset();
}

void
Editor::CheckIfObjectGotSelected(const glm::vec2& cursorPosition, bool groupSelect)
{
   auto CheckIfEditorObjectSelected =
      [this, cursorPosition](const std::vector< std::shared_ptr< EditorObject > >& objects) {
         auto newSelectedEditorObject = stl::find_if(objects, [cursorPosition](auto& object) {
            return object->IsVisible() && object->CheckIfCollidedScreenPosion(cursorPosition);
         });

         if (newSelectedEditorObject != objects.end())
         {
            HandleEditorObjectSelected(*newSelectedEditorObject);
         }

         return newSelectedEditorObject != objects.end();
      };

   if (not CheckIfEditorObjectSelected(editorObjects_)
       and not CheckIfEditorObjectSelected(animationPoints_))
   {
      auto newSelectedObject =
         m_currentLevel->GetGameObjectOnLocationAndLayer(cursorPosition, renderLayerToDraw_);

      if (newSelectedObject)
      {
         HandleGameObjectSelected(newSelectedObject->GetID(), groupSelect, false);
      }
   }
}

std::vector< Object::ID >
Editor::GetObjectsInArea(const std::array< glm::vec2, 4 >& area) const
{
   std::set< Object::ID > objectsList = {};

   const auto tiles = m_currentLevel->GetTilesFromRectangle(area);
   auto pathfinder = m_currentLevel->GetPathfinder();

   for (const auto& tile : tiles)
   {
      const auto& objectsOnNode = pathfinder.GetNodeFromTile(tile).objectsOnThisNode_;

      if (renderLayerToDraw_ != -1)
      {
         const auto gameObjectsOnNode = m_currentLevel->GetObjects(objectsOnNode);
         for (const auto& object : gameObjectsOnNode)
         {
            if (object->GetSprite().GetRenderInfo().layer == renderLayerToDraw_)
            {
               objectsList.insert(object->GetID());
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
   renderer::VulkanRenderer::SetupData();

   DrawGrid();
   renderer::VulkanRenderer::CreateLinePipeline();
   renderer::VulkanRenderer::SetupLineData();
   renderer::VulkanRenderer::UpdateLineData();
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
   return editorObjectSelected_ or currentEditorObjectSelected_ or gameObjectSelected_
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

            gui_.ObjectDeleted(currentEditorObjectSelected_->GetLinkedObjectID());
            if (Object::GetTypeFromID(currentEditorObjectSelected_->GetLinkedObjectID())
                == ObjectType::ANIMATION_POINT)
            {
               animationPoints_.erase(stl::find(animationPoints_, currentEditorObjectSelected_));
            }
            currentEditorObjectSelected_->DeleteLinkedObject();
         }
         else
         {
            UnselectGameObject(object, true);
            gui_.ObjectDeleted(object);

            m_currentLevel->DeleteObject(object);
         }
         break;
      default: {
      }
   }
}

void
Editor::Render(VkCommandBuffer cmdBuffer)
{
   if (m_levelLoaded)
   {
      auto& renderData =
         renderer::Data::renderData_[renderer::VulkanRenderer::GetCurrentlyBoundType()];

      m_currentLevel->GetSprite().Render();

      DrawEditorObjects();
      DrawAnimationPoints();

      m_currentLevel->RenderGameObjects();

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
            dynamic_cast< GameObject& >(m_currentLevel->GetObjectRef(currentSelectedGameObject_))
               .GetSprite()
               .GetRenderIdx();
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

         vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &renderData.vertexBuffer.at(idx), offsets.data());

         vkCmdBindIndexBuffer(cmdBuffer, renderData.indexBuffer.at(idx), 0, VK_INDEX_TYPE_UINT32);

         vkCmdDrawIndexed(cmdBuffer, numObjects * renderer::INDICES_PER_SPRITE, 1, 0, 0, 0);
      }

      // DRAW LINES
      vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                        renderer::EditorData::linePipeline_);

      vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &renderer::EditorData::lineVertexBuffer,
                             offsets.data());

      vkCmdBindIndexBuffer(cmdBuffer, renderer::EditorData::lineIndexBuffer, 0,
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

   EditorGUI::Render(cmdBuffer);
}

void
Editor::DrawEditorObjects()
{
   for (auto& object : editorObjects_)
   {
      object->Render();
   }
}

void
Editor::DrawAnimationPoints()
{
   bool drawLines = false;
   std::vector< Object::ID > animaltionPointIDs = {};
   glm::vec2 lineStart = {};

   if (currentSelectedGameObject_ != Object::INVALID_ID)
   {
      auto& baseObject = m_currentLevel->GetObjectRef(currentSelectedGameObject_);
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
      point->Render();

      if (drawLines)
      {
         if (std::ranges::find(animaltionPointIDs, point->GetLinkedObjectID())
             != animaltionPointIDs.end())
         {
            renderer::VulkanRenderer::DrawDynamicLine(lineStart, point->GetPosition());
            lineStart = point->GetCenteredPosition();
         }
      }
   }

   renderer::VulkanRenderer::UpdateLineData(renderer::EditorData::numGridLines);
}

void
Editor::DrawBoundingBoxes()
{
   auto drawBoundingBox = [](const renderer::Sprite& sprite) {
      const auto rect = sprite.GetTransformedRectangle();
      renderer::VulkanRenderer::DrawDynamicLine(rect[0], rect[1]);
      renderer::VulkanRenderer::DrawDynamicLine(rect[1], rect[2]);
      renderer::VulkanRenderer::DrawDynamicLine(rect[2], rect[3]);
      renderer::VulkanRenderer::DrawDynamicLine(rect[3], rect[0]);
   };

   for (const auto object : selectedObjects_)
   {
      drawBoundingBox(
         dynamic_cast< GameObject& >(m_currentLevel->GetObjectRef(object)).GetSprite());
   }

   if (currentSelectedGameObject_ != Object::INVALID_ID)
   {
      drawBoundingBox(
         dynamic_cast< GameObject& >(m_currentLevel->GetObjectRef(currentSelectedGameObject_))
            .GetSprite());
   }

   if (currentEditorObjectSelected_)
   {
      drawBoundingBox(currentEditorObjectSelected_->GetSprite());
   }
}

void
Editor::DrawGrid() const
{
   const auto levelSize = m_currentLevel->GetSize();
   const auto grad = gridCellSize_;

   const auto w = levelSize.x / grad;
   const auto h = levelSize.y / grad;
   // const auto offset = glm::ivec2(0, grad);

   for (int i = 0; i <= h; ++i)
   {
      renderer::VulkanRenderer::DrawLine(glm::vec2(0, i * grad), glm::vec2(levelSize.x, i * grad));
   }

   for (int i = 0; i <= w; ++i)
   {
      renderer::VulkanRenderer::DrawLine(glm::vec2(i * grad, 0), glm::vec2(i * grad, levelSize.y));
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
   if (m_levelLoaded)
   {
      if (currentEditorObjectSelected_)
      {
         UnselectEditorObject(currentEditorObjectSelected_->GetID());
      }
      if (currentSelectedGameObject_ != Object::INVALID_ID)
      {
         UnselectGameObject(currentSelectedGameObject_, false);
      }

      m_currentLevel.reset();
      editorObjects_.clear();
      pathfinderNodes_.clear();
      animationPoints_.clear();

      renderer::VulkanRenderer::FreeData(renderer::ApplicationType::EDITOR, false);
   }
}

void
Editor::CreateLevel(const std::string& name, const glm::ivec2& size)
{
   FreeLevelData();

   m_currentLevel = std::make_shared< Level >();
   m_currentLevel->Create(this, name, size);

   m_camera.Create(glm::vec3(0.0f, 0.0f, 0.0f), m_window->GetSize());
   m_camera.SetLevelSize(m_currentLevel->GetSize());

   m_levelLoaded = true;
   m_levelFileName = (LEVELS_DIR / (name + ".dgl")).string();
   gui_.LevelLoaded(m_currentLevel);

   m_currentLevel->GenerateTextureForCollision();

   SetupRendererData();
}

void
Editor::LoadLevel(const std::string& levelPath)
{
   FreeLevelData();

   renderer::VulkanRenderer::SetAppMarker(renderer::ApplicationType::EDITOR);

   {
      SCOPED_TIMER("Total level load");

      m_levelFileName = levelPath;
      m_currentLevel = std::make_shared< Level >();
      m_currentLevel->Load(this, levelPath);

      {
         SCOPED_TIMER("Animation points setup");

         const auto& gameObjects = m_currentLevel->GetObjects();
         for (const auto& object : gameObjects)
         {
            const auto animatablePtr = std::dynamic_pointer_cast< Animatable >(object);

            if (animatablePtr)
            {
               const auto& animationPoints = animatablePtr->GetAnimationKeypoints();

               for (const auto& point : animationPoints)
               {
                  auto editorObject = std::make_shared< EditorObject >(
                     *this, point.m_end, glm::vec2(20, 20), "NodeSprite.png", point.GetID());
                  editorObject->SetName(fmt::format("AnimationPoint{}", object->GetName()));
                  editorObject->SetVisible(false);
                  editorObject->Render();
                  animationPoints_.push_back(editorObject);
               }
            }
         }
      }

      m_camera.Create(glm::vec3(m_currentLevel->GetPlayer()->GetPosition(), 0.0f),
                      m_window->GetSize());
      m_camera.SetLevelSize(m_currentLevel->GetSize());

      m_levelLoaded = true;

      gui_.LevelLoaded(m_currentLevel);

      m_currentLevel->GenerateTextureForCollision();

      m_window->MakeFocus();
   }

   SetupRendererData();
}

void
Editor::SaveLevel(const std::string& levelPath)
{
   m_levelFileName = levelPath;
   m_currentLevel->Save(m_levelFileName);
}

void
Editor::AddGameObject(ObjectType objectType, const glm::vec2& position)
{
   HandleGameObjectSelected(m_currentLevel->AddGameObject(objectType, position)->GetID(), false,
                            false);
}

void
Editor::CopyGameObjects(const std::vector< Object::ID >& objectsToCopy)
{
   std::vector< Object::ID > newObjects = {};

   const auto mousePos = ScreenToGlobal(InputManager::GetMousePos());
   auto offset = glm::vec2{};
   for (const auto objectToCopy : objectsToCopy)
   {
      const auto& object = dynamic_cast< GameObject& >(m_currentLevel->GetObjectRef(objectToCopy));
      const auto position = object.GetPosition();

      // Set it only once
      if (offset == glm::vec2{})
      {
         offset = (mousePos - position);
      }

      auto newObject = m_currentLevel->AddGameObject(object.GetType(), position + offset);
      newObject->SetSize(object.GetSize());
      newObject->SetHasCollision(object.GetHasCollision());
      newObject->GetSprite().SetTextureFromFile(object.GetSprite().GetTextureName());
      newObject->GetSprite().Rotate(object.GetSprite().GetRotation());

      gui_.ObjectUpdated(newObject->GetID());
      newObjects.push_back(newObject->GetID());
   }
}

void
Editor::AddObject(ObjectType objectType)
{
   std::shared_ptr< EditorObject > newObject;
   if (objectType == ObjectType::ANIMATION_POINT)
   {
      if (currentSelectedGameObject_ == Object::INVALID_ID)
      {
         Logger::Warn("Added new Animation point without currently selected object!");
      }

      auto& baseObject = m_currentLevel->GetObjectRef(currentSelectedGameObject_);
      auto& animatable = dynamic_cast< Animatable& >(baseObject);
      auto& gameObject = dynamic_cast< GameObject& >(baseObject);
      auto newNode = animatable.CreateAnimationNode(
         currentSelectedGameObject_,
         gameObject.GetPosition() + static_cast< glm::vec2 >(gameObject.GetSize()));
      newObject = std::make_shared< EditorObject >(*this, newNode.m_end, glm::ivec2(20, 20),
                                                   "NodeSprite.png", newNode.GetID());

      animationPoints_.push_back(newObject);
      animatable.ResetAnimation();

      renderer::VulkanRenderer::UpdateBuffers();
      renderer::VulkanRenderer::CreateLinePipeline();
   }

   HandleEditorObjectSelected(newObject);
}

void
Editor::ToggleAnimateObject()
{
   if (animateGameObject_)
   {
      // TODO: This should be changed in future!
      auto& enemy =
         dynamic_cast< Enemy& >(m_currentLevel->GetObjectRef(currentSelectedGameObject_));
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
   m_currentLevel->Save(m_levelFileName);
   playGame_ = true;
}

void
Editor::LaunchGameLoop()
{
   renderer::VulkanRenderer::isLoaded_ = false;
   game_ = std::make_unique< Game >();
   game_->Init("GameInit.txt", false);
   game_->LoadLevel(m_levelFileName);

   // TODO: Create game-thread and run it inside
   game_->MainLoop();
   game_.reset();

   renderer::VulkanRenderer::FreeData(renderer::ApplicationType::GAME, true);
   playGame_ = false;

   renderer::VulkanRenderer::SetAppMarker(renderer::ApplicationType::EDITOR);
   // renderer::VulkanRenderer::SetupData();
}

// std::shared_ptr< EditorObject >
// Editor::GetEditorObjectByID(Object::ID ID)
// {
//    auto editorObject =
//       std::find_if(editorObjects_.begin(), editorObjects_.end(), [ID](const auto& object) {
//          return object->GetLinkedObjectID() == ID;
//       });

//    assert(editorObject != editorObjects_.end()); // NOLINT

//    return *editorObject;
// }

void
Editor::RenderNodes(bool render)
{
   renderPathfinderNodes_ = render;
   m_currentLevel->RenderPathfinder(render);
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
      dynamic_cast< Animatable& >(m_currentLevel->GetObjectRef(currentSelectedGameObject_));

   animatable.RenderAnimationSteps(render);
   SetVisibleAnimationPoints(animatable, render);
}

void
Editor::SetLockAnimationPoints(bool lock)
{
   dynamic_cast< Animatable& >(m_currentLevel->GetObjectRef(currentSelectedGameObject_))
      .LockAnimationSteps(lock);
}

void
Editor::Update()
{
   if (m_levelLoaded)
   {
      m_currentLevel->UpdateCollisionTexture();
   }

   HandleCamera();

   if (animateGameObject_ && currentSelectedGameObject_ != Object::INVALID_ID)
   {
      auto& objectBase = m_currentLevel->GetObjectRef(currentSelectedGameObject_);
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

   auto& renderData = renderer::VulkanRenderer::GetRenderData();
   renderData.viewMat = m_camera.GetViewMatrix();
   renderData.projMat = m_camera.GetProjectionMatrix();

   DrawBoundingBoxes();

   if (mouseDrag_ and selectRect_ != std::array< glm::vec2, 4 >{})
   {
      renderer::VulkanRenderer::DrawDynamicLine(selectRect_[0], selectRect_[1]);
      renderer::VulkanRenderer::DrawDynamicLine(selectRect_[1], selectRect_[2]);
      renderer::VulkanRenderer::DrawDynamicLine(selectRect_[2], selectRect_[3]);
      renderer::VulkanRenderer::DrawDynamicLine(selectRect_[3], selectRect_[0]);
   }

   {
      const time::ScopedTimer uiTImer(&uiTime_);
      gui_.UpdateUI();
   }
}

void
Editor::UpdateAnimationData()
{
   dynamic_cast< Animatable& >(m_currentLevel->GetObjectRef(currentSelectedGameObject_))
      .UpdateAnimationData();
}

glm::vec2
Editor::GetWindowSize() const
{
   return m_window->GetSize();
}

const glm::mat4&
Editor::GetProjection() const
{
   return m_camera.GetProjectionMatrix();
}

const glm::mat4&
Editor::GetViewMatrix() const
{
   return m_camera.GetViewMatrix();
}

float
Editor::GetZoomLevel() const
{
   return m_camera.GetZoomLevel();
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
      m_timer.ToggleTimer();
      singleFrameTimer += m_timer.GetMicroDeltaTime();

      while (IsRunning() and (singleFrameTimer.count() >= TARGET_TIME_MICRO))
      {
         const time::ScopedTimer frameTimer(&timeLastFrame_);

         deltaTime_ = m_timer.GetMsDeltaTime();
         InputManager::PollEvents();

         // Run all deffered work units
         workQueue_.RunWorkUnits();

         HandleCamera();
         Update();

         if (windowInFocus_)
         {
            const time::ScopedTimer renderTimer(&renderTime_);
            renderer::VulkanRenderer::Render(this);
         }

         timeLastFrame_ = watch.Stop();

         if (m_frameTimer > 1.0f)
         {
            m_framesLastSecond = m_frames;
            m_frameTimer = 0.0f;
            m_frames = 0;
         }

         ++m_frames;
         m_frameTimer += TARGET_TIME_S;

         renderer::EditorData::curDynLineIdx = 0;
         if (m_levelLoaded and m_currentLevel->GetPathfinder().IsInitialized())
         {
            m_currentLevel->GetPathfinder().ClearPerFrameData();
         }

         if (playGame_)
         {
            LaunchGameLoop();
         }

         m_timer.ToggleTimer();
      }
   }
}

} // namespace looper
