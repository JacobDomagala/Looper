#include "editor.hpp"
#include "enemy.hpp"
#include "game.hpp"
#include "input_manager.hpp"
#include "renderer/renderer.hpp"
#include "renderer/sprite.hpp"
#include "renderer/vulkan_common.hpp"
#include "renderer/window/window.hpp"
#include "utils/file_manager.hpp"
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
   m_window = std::make_unique< renderer::Window >(screenSize.x, screenSize.y, "Editor");

   InputManager::Init(m_window->GetWindowHandle());
   InputManager::RegisterForKeyInput(this);
   InputManager::RegisterForMouseScrollInput(this);
   InputManager::RegisterForMouseButtonInput(this);
   InputManager::RegisterForMouseMovementInput(this);

   renderer::VulkanRenderer::Initialize(m_window->GetWindowHandle(),
                                        renderer::ApplicationType::EDITOR);
   gui_.Init();

   m_deltaTime = Timer::milliseconds(static_cast< long >(TARGET_TIME * 1000.0f));
}

void
Editor::ShowCursor(bool choice)
{
   m_window->ShowCursor(choice);
}

void
Editor::HandleCamera()
{
   m_timer.ToggleTimer();
   m_deltaTime = m_timer.GetMsDeltaTime();

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
Editor::KeyCallback(const KeyEvent& event)
{
   if (event.m_action == GLFW_PRESS)
   {
      if (event.m_key == GLFW_KEY_ESCAPE)
      {
         ActionOnObject(ACTION::UNSELECT);
      }

      if (event.m_key == GLFW_KEY_DELETE)
      {
         ActionOnObject(ACTION::REMOVE);
      }
   }
   else if (event.m_action == GLFW_RELEASE)
   {
      if (m_gameObjectSelected && event.m_key == GLFW_KEY_C)
      {
         Logger::Info("Copy object!");
         m_copiedGameObject = m_currentSelectedGameObject;
      }
      if (m_copiedGameObject && event.m_key == GLFW_KEY_V)
      {
         Logger::Info("Paste object!");
         CopyGameObject(m_copiedGameObject);
      }
   }
}

void
Editor::MouseScrollCallback(const MouseScrollEvent& event)
{
   if (!m_playGame && !EditorGUI::IsBlockingEvents() && m_levelLoaded)
   {
      m_camera.Zoom(static_cast< float >(event.m_xOffset + event.m_yOffset));
   }
}

void
Editor::MouseButtonCallback(const MouseButtonEvent& event)
{
   if (!m_playGame && !EditorGUI::IsBlockingEvents() && m_levelLoaded)
   {
      const auto mousePressed = event.m_action == GLFW_PRESS;
      m_mousePressedLastUpdate = mousePressed;

      if (mousePressed)
      {
         CheckIfObjectGotSelected(InputManager::GetMousePos());
      }
      else
      {
         // Object movement finished
         ShowCursor(true);
         SetMouseOnObject();

         m_movementOnEditorObject = false;
         m_movementOnGameObject = false;
         m_mouseDrag = false;
      }
   }
}

void
Editor::CursorPositionCallback(const CursorPositionEvent& event)
{
   if (!m_playGame && !EditorGUI::IsBlockingEvents() && m_levelLoaded)
   {
      const auto currentCursorPosition = glm::vec2(event.m_xPos, event.m_yPos);

      if (m_mousePressedLastUpdate)
      {
         ShowCursor(false);
         HandleMouseDrag(currentCursorPosition, currentCursorPosition - m_lastCursorPosition);
      }
      else
      {
         ShowCursor(true);
      }

      m_lastCursorPosition = currentCursorPosition;
   }
}

void
Editor::HandleMouseDrag(const glm::vec2& currentCursorPos, const glm::vec2& axis)
{
   // Rotate camera (or currently selected Object)
   if (InputManager::CheckKeyPressed(GLFW_KEY_LEFT_CONTROL))
   {
      // Calculate the value of cursor movement
      // For example:
      // - cursor was moved to the right then movementVector.x is positive, negative otherwise
      // - cursor was moved to the top of window then movementVector.y is positive, negative
      // otherwise
      const auto movementVector = currentCursorPos - m_lastCursorPosition;

      // Compute the bigger factor (magnitute hence abs())
      // Go with `-movementVector.x` because it feels right
      const auto movementVal = std::abs(movementVector.x) > std::abs(movementVector.y)
                                  ? -movementVector.x
                                  : movementVector.y;

      constexpr auto maxRotationAngle = 0.02f;

      const auto angle = glm::clamp(movementVal, -maxRotationAngle, maxRotationAngle);

      if (m_movementOnEditorObject || m_movementOnGameObject)
      {
         // Editor objects selected have higher priority of movement
         // for example when animation point is selected and it's placed on top of game object
         if (m_movementOnEditorObject)
         {
            m_currentEditorObjectSelected->Rotate(-angle, true);
            gui_.ObjectUpdated(m_currentEditorObjectSelected->GetLinkedObjectID());
         }
         else
         {
            m_currentSelectedGameObject->Rotate(-angle, true);
            gui_.ObjectUpdated(m_currentSelectedGameObject->GetID());
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
      const auto moveBy = glm::vec3(axis, 0.0f);

      if (m_movementOnEditorObject || m_movementOnGameObject)
      {
         // Editor objects selected have higher priority of movement
         // for example when animation point is selected and it's placed on top of game object
         if (m_movementOnEditorObject)
         {
            m_currentEditorObjectSelected->Move(m_camera.ConvertToCameraVector(moveBy));
            gui_.ObjectUpdated(m_currentEditorObjectSelected->GetLinkedObjectID());
         }
         else
         {
            m_currentSelectedGameObject->Move(m_camera.ConvertToCameraVector(moveBy));
            m_currentSelectedGameObject->GetSprite().SetInitialPosition(
               m_currentSelectedGameObject->GetPosition());
            gui_.ObjectUpdated(m_currentSelectedGameObject->GetID());
            auto animatable = std::dynamic_pointer_cast< Animatable >(m_currentSelectedGameObject);
            if (animatable)
            {
               animatable->SetAnimationStartLocation(m_currentSelectedGameObject->GetPosition());
               UpdateAnimationData();
            }
         }
      }
      else
      {
         m_camera.Move(moveBy);
      }
   }

   m_mouseDrag = true;
}

void
Editor::SetMouseOnObject()
{
   if (m_mouseDrag && (m_movementOnEditorObject || m_movementOnGameObject))
   {
      if (m_movementOnEditorObject)
      {
         InputManager::SetMousePos(m_currentEditorObjectSelected->GetScreenPositionPixels());
      }
      else if (m_movementOnGameObject)
      {
         InputManager::SetMousePos(m_currentSelectedGameObject->GetScreenPositionPixels());
      }
   }
}

void
Editor::HandleGameObjectSelected(const std::shared_ptr< GameObject >& newSelectedGameObject,
                                 bool fromGUI)
{
   if (m_currentSelectedGameObject != newSelectedGameObject)
   {
      if (m_currentSelectedGameObject)
      {
         // unselect previously selected object
         UnselectGameObject();
      }

      m_currentSelectedGameObject = newSelectedGameObject;

      // Make sure to render animation points if needed
      auto animatePtr = std::dynamic_pointer_cast< Animatable >(m_currentSelectedGameObject);

      if (animatePtr && animatePtr->GetRenderAnimationSteps())
      {
         SetRenderAnimationPoints(true);
      }

      // mark new object as selected
      SelectGameObject();
      m_gameObjectSelected = true;

      if (m_editorObjectSelected)
      {
         UnselectEditorObject();
      }

      gui_.GameObjectSelected(m_currentSelectedGameObject);
   }

   m_movementOnGameObject = !fromGUI;
}

void
Editor::HandleObjectSelected(Object::ID objectID, bool fromGUI)
{
   switch (Object::GetTypeFromID(objectID))
   {
      case ObjectType::ANIMATION_POINT: {
         auto it = std::ranges::find_if(animationPoints_, [objectID](const auto& point) {
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
         auto it = std::find_if(m_editorObjects.begin(), m_editorObjects.end(),
                                [objectID](const auto& editorObject) {
                                   return editorObject->GetLinkedObjectID() == objectID;
                                });

         if (it != m_editorObjects.end())
         {
            HandleEditorObjectSelected(*it, fromGUI);
         }
      }
      break;

      default: {
      }
   }
}

void
Editor::SelectGameObject()
{
   // m_currentSelectedGameObject->SetColor({1.0f, 1.0f, 1.0f, 0.0f});
}

void
Editor::UnselectGameObject()
{
   m_gameObjectSelected = false;
   m_movementOnGameObject = false;
   gui_.GameObjectUnselected();
   auto animatablePtr = std::dynamic_pointer_cast< Animatable >(m_currentSelectedGameObject);
   if (animatablePtr and animatablePtr->GetRenderAnimationSteps())
   {
      const auto& animationPoints = animatablePtr->GetAnimationKeypoints();

      for (auto& animationPoint : animationPoints)
      {
         auto it = std::ranges::find_if(animationPoints_, [&animationPoint](auto& editorObject) {
            return editorObject->GetLinkedObjectID() == animationPoint.GetID();
         });

         if (it != animationPoints_.end())
         {
            (*it)->SetVisible(false);
         }
      }
   }

   if (m_currentSelectedGameObject)
   {
      // m_currentSelectedGameObject->SetColor({1.0f, 1.0f, 1.0f, 1.0f});
      // m_currentSelectedGameObject->GetSprite().SetModifiers(
      //   renderer::Sprite::Modifiers{glm::vec2{1.0f, 1.0f}});
      m_currentSelectedGameObject.reset();
   }
}

void
Editor::HandleEditorObjectSelected(const std::shared_ptr< EditorObject >& newSelectedEditorObject,
                                   bool fromGUI)
{
   if (m_editorObjectSelected && (newSelectedEditorObject != m_currentEditorObjectSelected))
   {
      UnselectEditorObject();
   }

   m_currentEditorObjectSelected = newSelectedEditorObject;
   m_editorObjectSelected = true;
   m_movementOnEditorObject = !fromGUI;

   m_currentEditorObjectSelected->SetObjectSelected();
   gui_.EditorObjectSelected(newSelectedEditorObject);
}

void
Editor::UnselectEditorObject()
{
   gui_.EditorObjectUnselected();
   m_editorObjectSelected = false;
   m_movementOnEditorObject = false;
   if (m_currentEditorObjectSelected)
   {
      m_currentEditorObjectSelected->SetObjectUnselected();
      m_currentEditorObjectSelected.reset();
   }
}

void
Editor::CheckIfObjectGotSelected(const glm::vec2& cursorPosition)
{
   auto CheckIfEditorObjectSelected =
      [this, cursorPosition](const std::vector< std::shared_ptr< EditorObject > >& objects) {
         auto newSelectedEditorObject =
            std::find_if(objects.begin(), objects.end(), [cursorPosition](auto& object) {
               return object->IsVisible() && object->CheckIfCollidedScreenPosion(cursorPosition);
            });

         if (newSelectedEditorObject != objects.end())
         {
            HandleEditorObjectSelected(*newSelectedEditorObject);
         }

         return newSelectedEditorObject != objects.end();
      };


   if (CheckIfEditorObjectSelected(m_editorObjects)
       or CheckIfEditorObjectSelected(animationPoints_))
   {
      return;
   }

   auto newSelectedObject = m_currentLevel->GetGameObjectOnLocation(cursorPosition);

   if (newSelectedObject)
   {
      HandleGameObjectSelected(newSelectedObject);
   }
}

void
Editor::SetupRendererData()
{
   renderer::VulkanRenderer::SetupData();

   DrawGrid();
   renderer::VulkanRenderer::CreateLinePipeline();
   renderer::VulkanRenderer::SetupLineData();
   renderer::VulkanRenderer::UpdateLineData();

   renderer::VulkanRenderer::SetupEditorData(ObjectType::PATHFINDER_NODE);
   renderer::VulkanRenderer::SetupEditorData(ObjectType::ANIMATION_POINT);
}

void
Editor::ActionOnObject(Editor::ACTION action)
{
   switch (action)
   {
      case ACTION::UNSELECT:

         if (m_editorObjectSelected && m_currentEditorObjectSelected)
         {
            UnselectEditorObject();
         }
         else if (m_gameObjectSelected && m_currentSelectedGameObject)
         {
            UnselectGameObject();
         }
         break;

      case ACTION::REMOVE:

         if (m_editorObjectSelected && m_currentEditorObjectSelected)
         {
            gui_.ObjectDeleted(m_currentEditorObjectSelected->GetLinkedObjectID());
            m_editorObjects.erase(std::find(m_editorObjects.begin(), m_editorObjects.end(),
                                            m_currentEditorObjectSelected));
            m_currentEditorObjectSelected->DeleteLinkedObject();
            UnselectEditorObject();
         }
         else if (m_gameObjectSelected && m_currentSelectedGameObject)
         {
            if (m_currentSelectedGameObject->GetType() == ObjectType::PLAYER)
            {
               m_player.reset();
            }
            else
            {
               m_currentLevel->DeleteObject(m_currentSelectedGameObject->GetID());
            }

            gui_.ObjectDeleted(m_currentSelectedGameObject->GetID());

            UnselectGameObject();
         }
         break;
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

      vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                        renderer::Data::graphicsPipeline_);

      auto offsets = std::to_array< const VkDeviceSize >({0});
      vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &renderData.vertexBuffer, offsets.data());

      vkCmdBindIndexBuffer(cmdBuffer, renderData.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

      vkCmdBindDescriptorSets(
         cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer::Data::pipelineLayout_, 0, 1,
                              &renderData.descriptorSets[renderer::Data::currentFrame_], 0, nullptr);

      renderer::PushConstants pushConstants = {};
      pushConstants.selectedIdx = -1.0f;

      if (m_currentSelectedGameObject)
      {
         const auto tmpIdx = m_currentSelectedGameObject->GetSprite().GetRenderIdx();
         pushConstants.selectedIdx = static_cast< float >(tmpIdx);
      }

      vkCmdPushConstants(cmdBuffer, renderer::Data::pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0,
                         sizeof(renderer::PushConstants), &pushConstants);

      const auto numObjects =
         renderData.numMeshes - renderer::EditorData::numNodes_ - renderer::EditorData::numPoints_;
      vkCmdDrawIndexed(cmdBuffer, numObjects * renderer::INDICES_PER_SPRITE, 1, 0, 0, 0);

      if (m_currentSelectedGameObject)
      {
         const auto tmpIdx = m_currentSelectedGameObject->GetSprite().GetRenderIdx();
         pushConstants.selectedIdx = -1.0f;
         vkCmdPushConstants(cmdBuffer, renderer::Data::pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT,
                            0, sizeof(renderer::PushConstants), &pushConstants);
         vkCmdDrawIndexed(cmdBuffer, 6, 1, tmpIdx * 6, 0, 0);
      }

      // DRAW PATHFINDER NODES
      if (m_renderPathfinderNodes)
      {
         vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &renderer::EditorData::pathfinderVertexBuffer,
                                offsets.data());

         vkCmdBindIndexBuffer(cmdBuffer, renderer::EditorData::pathfinderIndexBuffer, 0,
                              VK_INDEX_TYPE_UINT32);

         vkCmdDrawIndexed(cmdBuffer, renderer::EditorData::numNodes_ * renderer::INDICES_PER_SPRITE,
                          1, 0, 0, 0);
      }


      if (renderer::EditorData::numPoints_)
      {
         // DRAW ANIMATION POINTS
         vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &renderer::EditorData::animationVertexBuffer,
                                offsets.data());

         vkCmdBindIndexBuffer(cmdBuffer, renderer::EditorData::animationIndexBuffer, 0,
                              VK_INDEX_TYPE_UINT32);

         vkCmdDrawIndexed(
            cmdBuffer, renderer::EditorData::numPoints_ * renderer::INDICES_PER_SPRITE, 1, 0, 0, 0);
      }


      // DRAW LINES
      vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer::Data::linePipeline_);

      vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &renderer::Data::lineVertexBuffer, offsets.data());

      vkCmdBindIndexBuffer(cmdBuffer, renderer::Data::lineIndexBuffer, 0, VK_INDEX_TYPE_UINT32);

      vkCmdBindDescriptorSets(
         cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer::Data::linePipelineLayout_, 0, 1,
         &renderer::Data::lineDescriptorSets_[renderer::Data::currentFrame_], 0, nullptr);

      renderer::LinePushConstants linePushConstants = {};
      linePushConstants.color = glm::vec4(0.4f, 0.5f, 0.6f, static_cast< float >(m_drawGrid));

      vkCmdPushConstants(cmdBuffer, renderer::Data::linePipelineLayout_,
                         VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(renderer::LinePushConstants),
                         &linePushConstants);

      vkCmdDrawIndexed(cmdBuffer, renderer::Data::numGridLines * renderer::INDICES_PER_LINE, 1, 0,
                       0, 0);

      linePushConstants.color = glm::vec4(0.5f, 0.8f, 0.8f, 1.0f);
      vkCmdPushConstants(cmdBuffer, renderer::Data::linePipelineLayout_,
                         VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(renderer::LinePushConstants),
                         &linePushConstants);
      vkCmdDrawIndexed(cmdBuffer, renderer::Data::curDynLineIdx, 1,
                       renderer::Data::numGridLines * renderer::INDICES_PER_LINE, 0, 0);
   }

   EditorGUI::Render(cmdBuffer);
}

void
Editor::DrawBackgroundObjects()
{
   for (auto& object : m_editorObjects)
   {
      if (object->IsVisible() && object->GetIsBackground())
      {
         object->Render();
      }
   }
}

void
Editor::DrawEditorObjects()
{
   for (auto& object : m_editorObjects)
   {
      object->Render();
   }

   if (m_renderPathfinderNodes)
   {
      const auto& changedNodes = m_currentLevel->GetPathfinder().GetNodesModifiedLastFrame();

      for (auto nodeID : changedNodes)
      {
         pathfinderNodes_.at(static_cast< size_t >(nodeID))->Render();
      }
   }
}

void
Editor::DrawAnimationPoints()
{
   bool drawLines = false;
   std::vector< Object::ID > animaltionPointIDs = {};
   glm::vec2 lineStart = {};

   if (m_currentSelectedGameObject)
   {
      auto animatablePtr = std::dynamic_pointer_cast< Animatable >(m_currentSelectedGameObject);
      if (animatablePtr && animatablePtr->GetRenderAnimationSteps())
      {
         drawLines = true;

         const auto& animationPoints = animatablePtr->GetAnimationKeypoints();
         std::ranges::transform(
            animationPoints, std::back_inserter(animaltionPointIDs),
            [](const auto& animationKeyPoint) { return animationKeyPoint.GetID(); });

         lineStart = animatablePtr->GetAnimationStartLocation();
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

   renderer::VulkanRenderer::UpdateLineData(renderer::Data::numGridLines);
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

    if (m_currentSelectedGameObject)
    {
       drawBoundingBox(m_currentSelectedGameObject->GetSprite());
    }

    if (m_currentEditorObjectSelected)
    {
       drawBoundingBox(m_currentEditorObjectSelected->GetSprite());
    }
 }

void
Editor::DrawGrid()
{
   const auto levelSize = m_currentLevel->GetSize();
   const auto grad = m_gridCellSize;

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

   renderer::Data::numGridLines = renderer::Data::numLines;
}

void
Editor::SetGridData(bool render, int32_t cellSize)
{
   m_drawGrid = render;
   m_gridCellSize = cellSize;
}

std::pair< bool, int32_t >
Editor::GetGridData() const
{
   return {m_drawGrid, m_gridCellSize};
}

time::TimeStep
Editor::GetRenderTime() const
{
   return timeLastFrame_;
}

std::pair< uint32_t, uint32_t >
Editor::GetRenderOffsets() const
{
   return {numObjects_, numPathfinderNodes_};
}

void
Editor::SetupPathfinderNodes()
{
   const auto& pathfinderNodes = m_currentLevel->GetPathfinder().GetAllNodes();
   std::ranges::transform(
      pathfinderNodes, std::back_inserter(pathfinderNodes_), [this](const auto& node) {
         const auto tileSize = m_currentLevel->GetTileSize();

         auto pathfinderNode = std::make_shared< EditorObject >(
            *this, node.m_position, glm::ivec2(tileSize, tileSize), "white.png", node.GetID());

         pathfinderNode->Render();

         return pathfinderNode;
      });
}

void
Editor::CreateLevel(const std::string& name, const glm::ivec2& size)
{
   if (m_levelLoaded)
   {
      UnselectEditorObject();
      UnselectGameObject();
      m_currentLevel.reset();
      m_editorObjects.clear();
      pathfinderNodes_.clear();
   }

   m_currentLevel = std::make_shared< Level >();
   m_currentLevel->Create(this, name, size);

   SetupPathfinderNodes();

   m_camera.Create(glm::vec3(0.0f, 0.0f, 0.0f), m_window->GetSize());
   m_camera.SetLevelSize(m_currentLevel->GetSize());

   m_levelLoaded = true;
   m_levelFileName = (LEVELS_DIR / (name + ".dgl")).string();
   gui_.LevelLoaded(m_currentLevel);

   SetupRendererData();
}

void
Editor::LoadLevel(const std::string& levelPath)
{
   if (m_levelLoaded)
   {
      UnselectEditorObject();
      UnselectGameObject();
      m_currentLevel.reset();
      m_editorObjects.clear();
      pathfinderNodes_.clear();
   }

   renderer::VulkanRenderer::SetAppMarker(renderer::ApplicationType::EDITOR);

   m_levelFileName = levelPath;
   m_currentLevel = std::make_shared< Level >();
   m_currentLevel->Load(this, levelPath);

   SetupPathfinderNodes();

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

   m_camera.Create(glm::vec3(m_currentLevel->GetPlayer()->GetPosition(), 0.0f),
                   m_window->GetSize());
   m_camera.SetLevelSize(m_currentLevel->GetSize());

   m_levelLoaded = true;
   gui_.LevelLoaded(m_currentLevel);
   SetupRendererData();
}

void
Editor::SaveLevel(const std::string& levelPath)
{
   m_levelFileName = levelPath;
   m_currentLevel->Save(m_levelFileName);
}

void
Editor::AddGameObject(ObjectType objectType)
{
   HandleGameObjectSelected(m_currentLevel->AddGameObject(objectType));
}

void
Editor::CopyGameObject(const std::shared_ptr< GameObject >& objectToCopy)
{
   // For now we only copy type/size/collision/sprite

   auto newObject = m_currentLevel->AddGameObject(objectToCopy->GetType());
   newObject->SetSize(objectToCopy->GetSize());
   newObject->SetHasCollision(objectToCopy->GetHasCollision());
   newObject->GetSprite().SetTextureFromFile(objectToCopy->GetSprite().GetTextureName());

   HandleGameObjectSelected(newObject);
}

void
Editor::AddObject(ObjectType objectType)
{
   std::shared_ptr< EditorObject > newObject;
   if (objectType == ObjectType::ANIMATION_POINT)
   {
      if (!m_currentSelectedGameObject)
      {
         Logger::Warn("Added new Animation point without currently selected object!");
      }
      auto animatablePtr = std::dynamic_pointer_cast< Animatable >(m_currentSelectedGameObject);
      auto newNode = animatablePtr->CreateAnimationNode(
         m_currentSelectedGameObject->GetID(),
         m_currentSelectedGameObject->GetPosition()
            + static_cast< glm::vec2 >(m_currentSelectedGameObject->GetSize()));
      newObject = std::make_shared< EditorObject >(*this, newNode.m_end, glm::ivec2(20, 20),
                                                   "NodeSprite.png", newNode.GetID());

      animationPoints_.push_back(newObject);
      animatablePtr->ResetAnimation();

      renderer::VulkanRenderer::SetupEditorData(ObjectType::ANIMATION_POINT);
   }

   HandleEditorObjectSelected(newObject);
}

void
Editor::ToggleAnimateObject()
{
   if (m_animateGameObject)
   {
      auto enemyPtr = std::dynamic_pointer_cast< Enemy >(m_currentSelectedGameObject);
      enemyPtr->SetPosition(enemyPtr->GetInitialPosition());
      m_animateGameObject = false;
   }
   else
   {
      m_animateGameObject = true;
   }
}

bool
Editor::IsObjectAnimated() const
{
   return m_animateGameObject;
}

void
Editor::PlayLevel()
{
   // TODO: For future we'd want to check if anything got changed,
   //       so we don't always save (this can get costly later!)
   m_currentLevel->Save(m_levelFileName);
   m_playGame = true;
}

void
Editor::LaunchGameLoop()
{
   m_game = std::make_unique< Game >();
   m_game->Init("GameInit.txt", false);
   m_game->LoadLevel(m_levelFileName);

   // TODO: Create game-thread and run it inside
   m_game->MainLoop();
   m_game.reset();

   renderer::VulkanRenderer::FreeData();
   m_playGame = false;

   renderer::VulkanRenderer::SetAppMarker(renderer::ApplicationType::EDITOR);
   // renderer::VulkanRenderer::SetupData();
}

// std::shared_ptr< EditorObject >
// Editor::GetEditorObjectByID(Object::ID ID)
// {
//    auto editorObject =
//       std::find_if(m_editorObjects.begin(), m_editorObjects.end(), [ID](const auto& object) {
//          return object->GetLinkedObjectID() == ID;
//       });

//    assert(editorObject != m_editorObjects.end()); // NOLINT

//    return *editorObject;
// }

void
Editor::RenderNodes(bool render)
{
   if (m_renderPathfinderNodes != render)
   {
      m_renderPathfinderNodes = render;

      // std::for_each(pathfinderNodes_.begin(), pathfinderNodes_.end(),
      //               [render](auto& node) { node->SetVisible(render); });
   }
}

bool
Editor::GetRenderNodes() const
{
   return m_renderPathfinderNodes;
}

void
Editor::SetRenderAnimationPoints(bool render)
{
   const auto animatablePtr = std::dynamic_pointer_cast< Animatable >(m_currentSelectedGameObject);

   if (animatablePtr)
   {
      animatablePtr->RenderAnimationSteps(render);
      const auto& animationPoints = animatablePtr->GetAnimationKeypoints();

      for (auto& animationPoint : animationPoints)
      {
         auto it =
            std::find_if(animationPoints_.begin(), animationPoints_.end(),
                         [&animationPoint](auto& editorObject) {
                            return editorObject->GetLinkedObjectID() == animationPoint.GetID();
                         });

         if (it != animationPoints_.end())
         {
            (*it)->SetVisible(render);
         }
      }
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

         std::for_each(m_editorObjects.begin(), m_editorObjects.end(), [render,
         currenltySelectedName](EditorObject& object) { if (object.GetName() == "Animationpoint" +
         currenltySelectedName)
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
      auto moveBy = std::dynamic_pointer_cast< Animatable >(m_currentSelectedGameObject)
                       ->SingleAnimate(m_deltaTime);

      if (glm::length(moveBy) > 0.0f)
      {
         m_currentSelectedGameObject->Move(moveBy);
      }
      else
      {
         m_animateGameObject = false;
      }
   }

   gui_.UpdateUI();

   renderer::VulkanRenderer::view_mat = m_camera.GetViewMatrix();
   renderer::VulkanRenderer::proj_mat = m_camera.GetProjectionMatrix();

   DrawBoundingBoxes();
}

void
Editor::UpdateAnimationData()
{
   const auto animatablePtr = std::dynamic_pointer_cast< Animatable >(m_currentSelectedGameObject);

   if (animatablePtr)
   {
      animatablePtr->UpdateAnimationData();
   }
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
   return m_isRunning;
}

void
Editor::MainLoop()
{
   time::Stopwatch watch;
   while (IsRunning())
   {
      watch.Start();

      m_window->Clear();

      InputManager::PollEvents();

      HandleCamera();
      Update();

      renderer::VulkanRenderer::Draw(this);

      timeLastFrame_ = watch.Stop();

      renderer::Data::curDynLineIdx = 0;
      if (m_levelLoaded and m_currentLevel->GetPathfinder().IsInitialized())
      {
         m_currentLevel->GetPathfinder().ClearPerFrameData();
      }


      if (m_playGame)
      {
         LaunchGameLoop();
      }
   }
}

} // namespace looper
