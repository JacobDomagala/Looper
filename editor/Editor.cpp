#include "Editor.hpp"
#include "EditorGUI.hpp"
#include "Enemy.hpp"
#include "utils/file_manager.hpp"
#include "Game.hpp"
#include "InputManager.hpp"
// #include "RenderCommand.hpp"
#include "renderer.hpp"
#include "Window.hpp"
#include "stopwatch.hpp"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <set>
#include <string>
#include <vulkan/vulkan.h>
#include "renderer/renderer.hpp"
#include "renderer/vulkan_common.hpp"

namespace looper {

Editor::Editor(const glm::ivec2& screenSize) : m_gui(*this)
{
   m_window = std::make_unique< Window >(screenSize.x, screenSize.y, "Editor");

   InputManager::Init(m_window->GetWindowHandle());
   InputManager::RegisterForKeyInput(this);
   InputManager::RegisterForMouseScrollInput(this);
   InputManager::RegisterForMouseButtonInput(this);
   InputManager::RegisterForMouseMovementInput(this);

   /*RenderCommand::Init();
   Renderer::Init();*/
   render::vulkan::VulkanRenderer::Initialize(m_window->GetWindowHandle());

   // m_gui.Init();
   render::vulkan::VulkanRenderer::CreateRenderPipeline();
   m_gui.Init();

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

      if (m_mousePressedLastUpdate && m_levelLoaded)
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

      constexpr auto maxRotationAngle = 0.025f;
      const auto angle = glm::clamp(axis.x > 0 ? movementVector.x : -movementVector.y,
                                    -maxRotationAngle, maxRotationAngle);

      if (m_movementOnEditorObject || m_movementOnGameObject)
      {
         // Editor objects selected have higher priority of movement
         // for example when animation point is selected and it's placed on top of game object
         if (m_movementOnEditorObject)
         {
            m_currentEditorObjectSelected->Rotate(-angle, true);
            m_gui.ObjectUpdated(m_currentEditorObjectSelected->GetLinkedObjectID());
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
      const auto& moveBy = glm::vec3(axis.x, axis.y, 0.0f);

      if (m_movementOnEditorObject || m_movementOnGameObject)
      {
         // Editor objects selected have higher priority of movement
         // for example when animation point is selected and it's placed on top of game object
         if (m_movementOnEditorObject)
         {
            m_currentEditorObjectSelected->Move(m_camera.ConvertToCameraVector(moveBy));
            m_gui.ObjectUpdated(m_currentEditorObjectSelected->GetLinkedObjectID());
         }
         else
         {
            m_currentSelectedGameObject->Move(m_camera.ConvertToCameraVector(moveBy));
            m_currentSelectedGameObject->GetSprite().SetInitialPosition(
               m_currentSelectedGameObject->GetPosition());
            m_gui.ObjectUpdated(m_currentSelectedGameObject->GetID());
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
         m_camera.Move(-moveBy);
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
Editor::HandleGameObjectSelected(const std::shared_ptr< GameObject >& newSelectedGameObject, bool fromGUI)
{
   if (m_currentSelectedGameObject != newSelectedGameObject)
   {
      if (m_currentSelectedGameObject)
      {
         // unselect previously selected object
         UnselectGameObject();
      }

      m_currentSelectedGameObject = newSelectedGameObject;

      // mark new object as selected
      SelectGameObject();
      m_gameObjectSelected = true;

      if (m_editorObjectSelected)
      {
         UnselectEditorObject();
      }

      m_gui.GameObjectSelected(m_currentSelectedGameObject);
   }

   m_movementOnGameObject = !fromGUI;
}

void
Editor::HandleObjectSelected(Object::ID objectID, bool fromGUI)
{
   auto it = std::find_if(m_editorObjects.begin(), m_editorObjects.end(),
                          [objectID](const auto& editorObject) {
                             return editorObject->GetLinkedObjectID() == objectID;
                          });

   if (it != m_editorObjects.end())
   {
      HandleEditorObjectSelected(*it, fromGUI);
   }
}

void
Editor::SelectGameObject()
{
   m_currentSelectedGameObject->SetColor({1.0f, 0.0f, 0.0f});
}

void
Editor::UnselectGameObject()
{
   m_gameObjectSelected = false;
   m_movementOnGameObject = false;
   m_gui.GameObjectUnselected();
   if (m_currentSelectedGameObject)
   {
      m_currentSelectedGameObject->SetColor({1.0f, 1.0f, 1.0f});
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
   m_gui.EditorObjectSelected(newSelectedEditorObject);
}

void
Editor::UnselectEditorObject()
{
   m_gui.EditorObjectUnselected();
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
   auto newSelectedEditorObject =
      std::find_if(m_editorObjects.begin(), m_editorObjects.end(), [cursorPosition](auto& object) {
         return object->IsVisible() && object->CheckIfCollidedScreenPosion(cursorPosition);
      });

   if (newSelectedEditorObject != m_editorObjects.end())
   {
      HandleEditorObjectSelected(*newSelectedEditorObject);
   }
   else
   {
      auto newSelectedObject = m_currentLevel->GetGameObjectOnLocation(cursorPosition);

      if (newSelectedObject)
      {
         HandleGameObjectSelected(newSelectedObject);
      }
   }
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
            m_gui.ObjectDeleted(m_currentEditorObjectSelected->GetLinkedObjectID());
            m_editorObjects.erase(std::find(m_editorObjects.begin(), m_editorObjects.end(),
                                            m_currentEditorObjectSelected));
            m_currentEditorObjectSelected->DeleteLinkedObject();
            UnselectEditorObject();
         }
         else if (m_gameObjectSelected && m_currentSelectedGameObject)
         {
            if (m_currentSelectedGameObject->GetType() == looper::Object::TYPE::PLAYER)
            {
               m_player.reset();
            }
            else
            {
               m_currentLevel->DeleteObject(m_currentSelectedGameObject->GetID());
            }

            m_gui.ObjectDeleted(m_currentSelectedGameObject->GetID());

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
      //render::vulkan::VulkanRenderer::BeginScene(/*m_camera*/);

      m_currentLevel->GetSprite().Render();
      DrawBackgroundObjects();
      m_currentLevel->RenderGameObjects();

      DrawEditorObjects();
      DrawAnimationPoints();
      //DrawBoundingBoxes();
      //DrawGrid();

      //render::vulkan::VulkanRenderer::EndScene();
   }

   m_gui.Render(cmdBuffer);
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
      // Animation points are handled in Editor::DrawAnimationPoints()
      if (object->IsVisible()
          && (Object::GetTypeFromID(object->GetLinkedObjectID()) != Object::TYPE::ANIMATION_POINT)
          && !object->GetIsBackground())
      {
         object->Render();
      }
   }
}

void
Editor::DrawAnimationPoints()
{
   if (m_currentSelectedGameObject)
   {
      auto animatePtr = std::dynamic_pointer_cast< Animatable >(m_currentSelectedGameObject);

      if (animatePtr && animatePtr->GetRenderAnimationSteps())
      {
         std::vector< Object::ID > animaltionPointIDs = {};
         const auto& animationPoints = animatePtr->GetAnimationKeypoints();
         std::transform(animationPoints.begin(), animationPoints.end(),
                        std::back_inserter(animaltionPointIDs),
                        [](const auto& animationKeyPoint) { return animationKeyPoint.GetID(); });

         auto lineStart = animatePtr->GetAnimationStartLocation();
         for (auto& object : m_editorObjects)
         {
            if (object->IsVisible())
            {
               auto it = std::find(animaltionPointIDs.begin(), animaltionPointIDs.end(),
                                   object->GetLinkedObjectID());
               if (it != animaltionPointIDs.end())
               {
                  // render::vulkan::VulkanRenderer::DrawLine(lineStart, object->GetPosition(), {1.0f, 0.0f, 1.0f, 1.0f});
                  lineStart = object->GetCenteredPosition();

                  object->Render();
               }
            }
         }
      }
   }
}

void
Editor::DrawBoundingBoxes()
{
   // constexpr glm::vec4 color = {1.0f, 0.2f, 0.1f, 1.0f};

   // auto drawBoundingBox = [color](const Sprite& sprite) {
   //    const auto rect = sprite.GetTransformedRectangle();
   //    Renderer::DrawLine(rect[0], rect[1], color);
   //    Renderer::DrawLine(rect[1], rect[2], color);
   //    Renderer::DrawLine(rect[2], rect[3], color);
   //    Renderer::DrawLine(rect[3], rect[0], color);
   // };

   // if (m_currentSelectedGameObject)
   // {
   //    drawBoundingBox(m_currentSelectedGameObject->GetSprite());
   // }

   // if (m_currentEditorObjectSelected)
   // {
   //    drawBoundingBox(m_currentEditorObjectSelected->GetSprite());
   // }
}

void
Editor::DrawGrid()
{
   // if (m_drawGrid)
   // {
   //    const auto levelSize = m_currentLevel->GetSize();
   //    const auto grad = m_gridCellSize;

   //    const auto w = levelSize.x / grad;
   //    const auto h = levelSize.y / grad;
   //    // const auto offset = glm::ivec2(0, grad);

   //    for (int i = 0; i <= h; ++i)
   //    {
   //       Renderer::DrawLine(glm::vec2(0, i * grad), glm::vec2(levelSize.x, i * grad),
   //                          {1.0f, 0.0f, 1.0f, 1.0f});
   //    }

   //    for (int i = 0; i <= w; ++i)
   //    {
   //       Renderer::DrawLine(glm::vec2(i * grad, 0), glm::vec2(i * grad, levelSize.y),
   //                          {1.0f, 0.0f, 1.0f, 1.0f});
   //    }
   // }
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

void
Editor::CreateLevel(const std::string& name, const glm::ivec2& size)
{
   if (m_levelLoaded)
   {
      UnselectEditorObject();
      UnselectGameObject();
      m_currentLevel.reset();
      m_editorObjects.clear();
   }

   m_currentLevel = std::make_shared< Level >();
   m_currentLevel->Create(this, name, size);
   m_currentLevel->LoadShaders("Editor");

   // Populate editor objects
   const auto& pathfinderNodes = m_currentLevel->GetPathfinder().GetAllNodes();
   std::transform(pathfinderNodes.begin(), pathfinderNodes.end(),
                  std::back_inserter(m_editorObjects), [this](const auto& node) {
                     const auto tileSize = m_currentLevel->GetTileSize();

                     auto pathfinderNode = std::make_shared< EditorObject >(
                        *this, node.m_position, glm::ivec2(tileSize, tileSize), "white.png",
                        node.GetID());

                     pathfinderNode->SetIsBackground(true);
                     pathfinderNode->SetVisible(m_renderPathfinderNodes);
                     pathfinderNode->SetColor(glm::vec3{1.0f, 1.0f, 1.0f});

                     return pathfinderNode;
                  });

   m_camera.Create(glm::vec3(m_currentLevel->GetLevelPosition(), 0.0f), m_window->GetSize());
   m_camera.SetLevelSize(m_currentLevel->GetSize());

   m_levelLoaded = true;
   m_levelFileName = (LEVELS_DIR / (name + ".dgl")).string();
   m_gui.LevelLoaded(m_currentLevel);
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
   }

   m_levelFileName = levelPath;
   m_currentLevel = std::make_shared< Level >();
   m_currentLevel->Load(this, levelPath);

   // Populate editor objects
   const auto& pathfinderNodes = m_currentLevel->GetPathfinder().GetAllNodes();
   std::transform(pathfinderNodes.begin(), pathfinderNodes.end(),
                  std::back_inserter(m_editorObjects), [this](const auto& node) {
                     const auto tileSize = m_currentLevel->GetTileSize();

                     auto pathfinderNode = std::make_shared< EditorObject >(
                        *this, node.m_position, glm::ivec2(tileSize, tileSize), "white.png",
                        node.GetID());

                     pathfinderNode->SetIsBackground(true);
                     pathfinderNode->SetVisible(m_renderPathfinderNodes);
                     pathfinderNode->SetColor(glm::vec3{1.0f, 1.0f, 1.0f});

                     return pathfinderNode;
                  });

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
               *this, point.m_end, glm::ivec2(20, 20), "NodeSprite.png", point.GetID());
            editorObject->SetName("Animationpoint" + object->GetName());

            m_editorObjects.push_back(editorObject);
         }
      }
   }

   m_camera.Create(glm::vec3(m_currentLevel->GetPlayer()->GetPosition(), 0.0f),
                   m_window->GetSize());
   m_camera.SetLevelSize(m_currentLevel->GetSize());

   m_levelLoaded = true;
   m_gui.LevelLoaded(m_currentLevel);

   render::vulkan::VulkanRenderer::SetupData();
}

void
Editor::SaveLevel(const std::string& levelPath)
{
   m_levelFileName = levelPath;
   m_currentLevel->Save(m_levelFileName);
}

void
Editor::AddGameObject(GameObject::TYPE objectType)
{
   HandleGameObjectSelected(m_currentLevel->AddGameObject(objectType));
}

void
Editor::CopyGameObject(const std::shared_ptr<GameObject>& objectToCopy)
{
   // For now we only copy type/size/collision/sprite

   auto newObject = m_currentLevel->AddGameObject(objectToCopy->GetType());
   newObject->SetSize(objectToCopy->GetSize());
   newObject->SetHasCollision(objectToCopy->GetHasCollision());
   newObject->GetSprite().SetTextureFromFile(objectToCopy->GetSprite().GetTextureName());

   HandleGameObjectSelected(newObject);
}

void
Editor::AddObject(Object::TYPE objectType)
{
   std::shared_ptr< EditorObject > newObject;
   if (objectType == Object::TYPE::ANIMATION_POINT)
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

      m_editorObjects.push_back(newObject);
      animatablePtr->ResetAnimation();
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
   // Clear rednerer data
   // Renderer::Shutdown();
   EditorGUI::Shutdown();

   m_game = std::make_unique< Game >();
   m_game->Init("GameInit.txt");
   m_game->LoadLevel(m_levelFileName);
   m_game->MainLoop();
   m_game.reset();

   m_playGame = false;

   // Reinitialize renderer
   // glfwMakeContextCurrent(m_window->GetWindowHandle());
   // RenderCommand::Init();
   render::vulkan::VulkanRenderer::Initialize(m_window->GetWindowHandle());

   m_gui.Init();
}

std::shared_ptr< EditorObject >
Editor::GetEditorObjectByID(Object::ID ID)
{
   auto editorObject =
      std::find_if(m_editorObjects.begin(), m_editorObjects.end(), [ID](const auto& object) {
         return object->GetLinkedObjectID() == ID;
      });

   assert(editorObject != m_editorObjects.end()); // NOLINT

   return *editorObject;
}

void
Editor::RenderNodes(bool render)
{
   if (m_renderPathfinderNodes != render)
   {
      m_renderPathfinderNodes = render;

      std::for_each(m_editorObjects.begin(), m_editorObjects.end(), [render](auto& object) {
         if (Object::GetTypeFromID(object->GetLinkedObjectID())
             == looper::Object::TYPE::PATHFINDER_NODE)
         {
            object->SetVisible(render);
         }
      });
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
      auto animationPoints = animatablePtr->GetAnimationKeypoints();

      for (auto& animationPoint : animationPoints)
      {
         auto it = std::find_if(
            m_editorObjects.begin(), m_editorObjects.end(), [&animationPoint](auto& editorObject) {
               return editorObject->GetLinkedObjectID() == animationPoint.GetID();
            });

         if (it != m_editorObjects.end())
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

   if (m_gui.UpdateUI())
   {
      render::vulkan::VulkanRenderer::CreateCommandBuffers(this);
   }

   render::vulkan::VulkanRenderer::view_mat = m_camera.GetViewMatrix();
   render::vulkan::VulkanRenderer::proj_mat = m_camera.GetProjectionMatrix();
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

      render::vulkan::VulkanRenderer::Draw(this);

      timeLastFrame_ = watch.Stop();

      if (m_playGame)
      {
         LaunchGameLoop();
      }
   }
}

} // namespace looper
