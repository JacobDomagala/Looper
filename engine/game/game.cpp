#include "game.hpp"
#include "enemy.hpp"
#include "utils/file_manager.hpp"
// #include "RenderCommand.hpp"
#include "renderer.hpp"
#include "renderer/vulkan_common.hpp"
#include "renderer/window/window.hpp"

#include <GLFW/glfw3.h>
#include <fstream>
#include <string>

namespace looper {

void
Game::MainLoop()
{
   if (currentLevel_)
   {
      auto singleFrameTimer = time::microseconds(0);

      while (IsRunning())
      {
         timer_.ToggleTimer();
         singleFrameTimer += timer_.GetMicroDeltaTime();

         while (IsRunning() and (singleFrameTimer.count() >= TARGET_TIME_MICRO))
         {
            const auto dt = time::milliseconds(
               TARGET_TIME_MS * static_cast< float >(time::Timer::AreTimersRunning()));
            InputManager::PollEvents();
            ProcessInput(dt);

            workQueue_.RunWorkUnits();
            if (windowInFocus_)
            {
               renderer::VulkanRenderer::Render(this);
            }

            if (frameTimer_ > 1.0f)
            {
               framesLastSecond_ = frames_;
               frameTimer_ = 0.0f;
               frames_ = 0;
            }

            // Increment frame count and frame timer
            ++frames_;
            frameTimer_ += TARGET_TIME_S;

            // Decrement frame timer for next frame
            singleFrameTimer -= time::microseconds(TARGET_TIME_MICRO);

            InputManager::PollEvents();
         }
      }
   }
   InputManager::UnregisterFromInput(window_->GetWindowHandle(), this);
}


void
Game::Init(const std::string& configFile, bool loadLevel)
{
   isGame_ = true;

   std::ifstream initFile((ASSETS_DIR / configFile).string());

   if (!initFile)
   {
      Logger::Fatal("Game: Can't open {}", (ASSETS_DIR / configFile).string());
   }

   window_ = std::make_unique< renderer::Window >(USE_DEFAULT_SIZE, "WindowTitle", true);
   window_->MakeFocus();

   renderer::VulkanRenderer::Initialize(window_->GetWindowHandle(),
                                        renderer::ApplicationType::GAME);

   while (!initFile.eof())
   {
      std::string tmp = {};
      initFile >> tmp;
      if (tmp == "Levels:")
      {
         while (initFile.peek() != '\n' && initFile.peek() != EOF)
         {
            initFile >> tmp;
            m_levels.push_back(tmp);
         }
      }
      else if (tmp == "Font:")
      {
         initFile >> tmp;
         // m_font.SetFont(tmp);
      }
   }

   initFile.close();

   InputManager::Init(window_->GetWindowHandle());
   InputManager::RegisterForInput(window_->GetWindowHandle(), this);

   if (loadLevel)
   {
      // LoadLevel(m_levels[0]);
      LoadLevel((LEVELS_DIR / "TestLevel" / "TestLevel.dgl").string());
   }

   m_state = GameState::GAME;
   m_initialized = true;
}

// glm::vec2
// Game::CheckBulletCollision(const glm::vec2& positon, float range)
//{
//    return currentLevel_->GetCollidedPosition(
//       positon, glm::clamp(positon + range, glm::vec2{0.0f, 0.0f},
//                           static_cast< glm::vec2 >(currentLevel_->GetSize())));
// }

void
Game::MoveGameObject(Object::ID gameObject, const glm::vec2& moveBy) const
{
   auto& object = dynamic_cast< GameObject& >(currentLevel_->GetObjectRef(gameObject));
   const auto fromPosition = object.GetCenteredPosition();
   const auto destination = fromPosition + moveBy;

   const auto actualMoveBy =
      currentLevel_->GetCollidedPosition(fromPosition, destination) - fromPosition;
   object.Move(actualMoveBy);
}

void
Game::KeyEvents() // NOLINT
{
   const auto floatDeltaTime = static_cast< float >(deltaTime_.count());
   // Camera movement is disabled
   const auto cameraMovement = 0.05f * floatDeltaTime;
   const auto playerMovement = 0.5f * floatDeltaTime;

   auto playerMoveBy = glm::vec2();
   auto cameraMoveBy = glm::vec2();

   m_reverse = InputManager::CheckKeyPressed(GLFW_KEY_LEFT_CONTROL);

   if (!m_reverse)
   {
      if (InputManager::CheckKeyPressed(GLFW_KEY_LEFT))
      {
         currentLevel_->MoveObjs(glm::vec2(2.0f, 0.0f));
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_RIGHT))
      {
         currentLevel_->MoveObjs(glm::vec2(-2.0f, 0.0f));
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_O))
      {
         // glEnable(GL_BLEND);
         // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_1))
      {
         window_->ShowCursor(true);
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_2))
      {
         window_->ShowCursor(false);
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_P))
      {
         // glDisable(GL_BLEND);
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_W))
      {
         playerMoveBy += glm::vec2(0, playerMovement);
         cameraMoveBy += glm::vec2(0, -cameraMovement);
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_S))
      {
         playerMoveBy += glm::vec2(0, -playerMovement);
         cameraMoveBy += glm::vec2(0, cameraMovement);
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_A))
      {
         playerMoveBy += glm::vec2(-playerMovement, 0);
         cameraMoveBy += glm::vec2(cameraMovement, 0);
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_D))
      {
         playerMoveBy += glm::vec2(playerMovement, 0);
         cameraMoveBy += glm::vec2(-cameraMovement, 0);
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_R))
      {
         time::Timer::PauseAllTimers();
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_T))
      {
         time::Timer::ResumeAllTimers();
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_SPACE))
      {
         CenterCameraOnPlayer();
      }

      if (glm::length(playerMoveBy) > 0.0f)
      {
         camera_.Move(glm::vec3{cameraMoveBy, 0.0f});
         MoveGameObject(currentLevel_->GetPlayer().GetID(), playerMoveBy);
      }
   }
}

void
Game::MouseEvents()
{
   // const auto playerPos = m_player->GetCenteredPosition();
   // const auto mousePos = ScreenToGlobal(InputManager::GetMousePos());
   // const auto collided = currentLevel_->GetCollidedPosition(playerPos, mousePos);

   // Renderer::DrawLine(m_player->GetCenteredPosition(), collided, {0.8f, 0.0f, 0.3f, 1.0f});

   if (!m_reverse)
   {
      // value to control how fast should camera move
      constexpr int32_t multiplier = 1;

      // cursor's position from center of the screen to trigger camera movement
      constexpr float borderValue = 0.5f;
      constexpr float modifier = 1.f;

      const auto cameraMovement = modifier * floorf(static_cast< float >(deltaTime_.count()));
      auto cameraMoveBy = glm::vec2();
      const auto cursor = window_->GetCursorNormalized();

      if (cursor.x > borderValue)
      {
         const float someX = (cursor.x - borderValue) * static_cast< float >(multiplier);
         cameraMoveBy += glm::vec2(cameraMovement * someX, 0.0f);
      }
      else if (cursor.x < -borderValue)
      {
         const float someX = (cursor.x + borderValue) * static_cast< float >(multiplier);
         cameraMoveBy += glm::vec2(cameraMovement * someX, 0.0f);
      }
      if (cursor.y > borderValue)
      {
         const float someY = (cursor.y - borderValue) * static_cast< float >(multiplier);
         cameraMoveBy += glm::vec2(0.0f, cameraMovement * someY);
      }
      else if (cursor.y < -borderValue)
      {
         const float someY = (cursor.y + borderValue) * static_cast< float >(multiplier);
         cameraMoveBy += glm::vec2(0.0f, cameraMovement * someY);
      }
      if (glm::length(glm::vec2(cameraMoveBy)) > 0.0f)
      {
         camera_.Move(glm::vec3(cameraMoveBy, 0.0f));
      }
   }
}

void
Game::UpdateGameState()
{
   currentLevel_->Update(m_reverse);
}

void
Game::RenderFirstPass()
{
   // m_frameBuffer.BeginDrawingToTexture();

   currentLevel_->Render();

   // m_frameBuffer.EndDrawingToTexture();
}

void
Game::RenderSecondPass()
{
   // m_frameBuffer.DrawFrameBuffer();

   /* RenderText(std::to_string(m_deltaTime.count()) + " ms",
               glm::vec2(static_cast< float >(-WIDTH / 2), static_cast< float >(-HEIGHT / 2) + 20),
      0.4f, glm::vec3(1.0f, 0.0f, 1.0f));*/
}

void
Game::LoadLevel(const std::string& pathToLevel)
{
   renderer::VulkanRenderer::SetAppMarker(renderer::ApplicationType::GAME);

   currentLevel_ = std::make_shared< Level >();
   currentLevel_->Load(this, pathToLevel);

   camera_.Create(glm::vec3(currentLevel_->GetPlayer().GetCenteredPosition(), 0.0f),
                  window_->GetSize());
   camera_.SetLevelSize(currentLevel_->GetSize());

   workQueue_.PushWorkUnit([this] { return windowInFocus_; },
                           [] { renderer::VulkanRenderer::SetupData(); });
}

glm::vec2
Game::GetWindowSize() const
{
   return window_->GetSize();
}

const glm::mat4&
Game::GetProjection() const
{
   return camera_.GetProjectionMatrix();
}

const glm::mat4&
Game::GetViewMatrix() const
{
   return camera_.GetViewMatrix();
}

float
Game::GetZoomLevel() const
{
   return camera_.GetZoomLevel();
}

glm::vec2
Game::GetCursor()
{
   return window_->GetCursor();
}

glm::vec2
Game::GetCursorScreenPosition()
{
   return window_->GetCursorScreenPosition(camera_.GetProjectionMatrix());
}

bool
Game::IsRunning() const
{
   return window_->IsRunning();
}

void
Game::ProcessInput(time::milliseconds deltaTime)
{
   deltaTime_ = deltaTime;

   MouseEvents();
   KeyEvents();
   HandleReverseLogic();
   UpdateGameState();

   auto& renderData =
      renderer::Data::renderData_.at(renderer::VulkanRenderer::GetCurrentlyBoundType());
   renderData.viewMat = camera_.GetViewMatrix();
   renderData.projMat = camera_.GetProjectionMatrix();
}

void
Game::KeyCallback(KeyEvent& event)
{
   if ((event.key_ == GLFW_KEY_ESCAPE) and (event.action_ == GLFW_PRESS))
   {
      currentLevel_->Quit();
      window_->ShutDown();

      event.handled_ = true;
   }
}

bool
Game::IsReverse() const
{
   return m_reverse;
}

void
Game::HandleReverseLogic()
{
   if (m_reverse)
   {
      if (m_frameCount == 0)
      {
         m_reverse = false;
      }
      else
      {
         --m_frameCount;
         auto pos = cameraPositions_.GetLastState();

         camera_.SetCameraAtPosition(pos);
      }
   }
   else
   {
      if (m_frameCount != (NUM_FRAMES_TO_SAVE - 1))
      {
         ++m_frameCount;
      }

      cameraPositions_.PushState(camera_.GetPosition());
   }
}

void
Game::Render(VkCommandBuffer cmdBuffer)
{
   RenderFirstPass();
   RenderSecondPass();

   auto& renderData =
      renderer::Data::renderData_.at(renderer::VulkanRenderer::GetCurrentlyBoundType());

   vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.pipeline);


   auto offsets = std::to_array< const VkDeviceSize >({0});

   vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.pipelineLayout, 0,
                           1, &renderData.descriptorSets[renderer::Data::currentFrame_], 0,
                           nullptr);

   for (int32_t layer = renderer::NUM_LAYERS - 1; layer >= 0; --layer)
   {
      const auto idx = static_cast< size_t >(layer);

      const auto& numObjects = renderData.numMeshes.at(idx);
      if (numObjects == 0)
      {
         continue;
      }

      vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &renderData.vertexBuffer.at(idx), offsets.data());

      vkCmdBindIndexBuffer(cmdBuffer, renderData.indexBuffer.at(idx), 0, VK_INDEX_TYPE_UINT32);

      // const auto numObjects =
      // renderer::VulkanRenderer::GetNumMeshes(renderer::ApplicationType::GAME); numObjects_ =
      // numObjects.second - numObjects.first;
      vkCmdDrawIndexed(cmdBuffer, renderData.numMeshes.at(idx) * renderer::INDICES_PER_SPRITE, 1, 0,
                       0, 0);
   }
}

} // namespace looper
