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
   if (m_currentLevel)
   {
      // timer in microseconds
      auto singleFrameTimer = 0.0f;

      while (IsRunning())
      {
         m_timer.ToggleTimer();
         singleFrameTimer += m_timer.GetFloatDeltaTime();

         while (IsRunning() and singleFrameTimer >= TARGET_TIME)
         {
            m_window->Clear();
            const auto dt = Timer::milliseconds(
               static_cast< long >(TARGET_TIME * static_cast< float >(Timer::AreTimersRunning())));
            ProcessInput(dt);

            renderer::VulkanRenderer::Render(this);
            if (m_frameTimer > 1.0f)
            {
               m_framesLastSecond = m_frames;
               m_frameTimer = 0.0f;
               m_frames = 0;
            }

            // Increment frame count and frame timer
            ++m_frames;
            m_frameTimer += TARGET_TIME;

            // Decrement frame timer for next frame
            singleFrameTimer -= TARGET_TIME;

            InputManager::PollEvents();
         }
      }
   }
   InputManager::UnregisterFromInput(this);
}


void
Game::Init(const std::string& configFile, bool loadLevel)
{
   m_isGame = true;

   std::ifstream initFile((ASSETS_DIR / configFile).string());

   if (!initFile)
   {
      Logger::Fatal("Game: Can't open {}", (ASSETS_DIR / configFile).string());
   }

   m_window = std::make_unique< renderer::Window >(WIDTH, HEIGHT, "WindowTitle");

   renderer::VulkanRenderer::Initialize(m_window->GetWindowHandle(),
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

   InputManager::Init(m_window->GetWindowHandle());
   InputManager::RegisterForInput(this);

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
//    return m_currentLevel->GetCollidedPosition(
//       positon, glm::clamp(positon + range, glm::vec2{0.0f, 0.0f},
//                           static_cast< glm::vec2 >(m_currentLevel->GetSize())));
// }

void
Game::MoveGameObject(GameObject* gameObject, const glm::vec2& moveBy) const
{
   const auto fromPosition = gameObject->GetCenteredPosition();
   const auto destination = fromPosition + moveBy;

   const auto actualMoveBy =
      m_currentLevel->GetCollidedPosition(fromPosition, destination) - fromPosition;
   gameObject->Move(actualMoveBy);
}

void
Game::KeyEvents() // NOLINT
{
   const auto floatDeltaTime = static_cast< float >(m_deltaTime.count());
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
         m_currentLevel->MoveObjs(glm::vec2(2.0f, 0.0f));
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_RIGHT))
      {
         m_currentLevel->MoveObjs(glm::vec2(-2.0f, 0.0f));
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_ESCAPE))
      {
         m_currentLevel->Quit();
         m_window->ShutDown();
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_O))
      {
         // glEnable(GL_BLEND);
         // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_1))
      {
         m_window->ShowCursor(true);
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_2))
      {
         m_window->ShowCursor(false);
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
         Timer::PauseAllTimers();
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_T))
      {
         Timer::ResumeAllTimers();
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_SPACE))
      {
         m_camera.SetCameraAtObject(m_player);
      }

      if (glm::length(playerMoveBy) > 0.0f)
      {
         m_camera.Move(glm::vec3{cameraMoveBy, 0.0f});
         MoveGameObject(m_player.get(), playerMoveBy);
      }
   }
}

void
Game::MouseEvents()
{
   // const auto playerPos = m_player->GetCenteredPosition();
   // const auto mousePos = ScreenToGlobal(InputManager::GetMousePos());
   // const auto collided = m_currentLevel->GetCollidedPosition(playerPos, mousePos);

   // Renderer::DrawLine(m_player->GetCenteredPosition(), collided, {0.8f, 0.0f, 0.3f, 1.0f});

   if (!m_reverse)
   {
      // value to control how fast should camera move
      constexpr int32_t multiplier = 1;

      // cursor's position from center of the screen to trigger camera movement
      constexpr float borderValue = 0.5f;
      constexpr float modifier = 1.f;

      const auto cameraMovement = modifier * floorf(static_cast< float >(m_deltaTime.count()));
      auto cameraMoveBy = glm::vec2();
      const auto cursor = m_window->GetCursorNormalized();

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
         m_camera.Move(glm::vec3(cameraMoveBy, 0.0f));
      }
   }
}

void
Game::UpdateGameState()
{
   m_currentLevel->Update(m_reverse);
}

void
Game::RenderFirstPass()
{
   // m_frameBuffer.BeginDrawingToTexture();

   m_currentLevel->Render();

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

   m_currentLevel = std::make_shared< Level >();
   m_currentLevel->Load(this, pathToLevel);
   m_player = m_currentLevel->GetPlayer();

   m_camera.Create(glm::vec3(m_player->GetCenteredPosition(), 0.0f), m_window->GetSize());
   m_camera.SetLevelSize(m_currentLevel->GetSize());

   renderer::VulkanRenderer::SetupData();
}

glm::vec2
Game::GetWindowSize() const
{
   return m_window->GetSize();
}

const glm::mat4&
Game::GetProjection() const
{
   return m_camera.GetProjectionMatrix();
}

const glm::mat4&
Game::GetViewMatrix() const
{
   return m_camera.GetViewMatrix();
}

float
Game::GetZoomLevel() const
{
   return m_camera.GetZoomLevel();
}

glm::vec2
Game::GetCursor()
{
   return m_window->GetCursor();
}

glm::vec2
Game::GetCursorScreenPosition()
{
   return m_window->GetCursorScreenPosition(m_camera.GetProjectionMatrix());
}

bool
Game::IsRunning() const
{
   return m_window->IsRunning();
}

void
Game::RegisterForKeyInput(InputListener* listener)
{
   InputManager::RegisterForKeyInput(listener);
}

void
Game::ProcessInput(Timer::milliseconds deltaTime)
{
   m_deltaTime = deltaTime;

   MouseEvents();
   KeyEvents();

   HandleReverseLogic();
   UpdateGameState();

   auto& renderData =
      renderer::Data::renderData_.at(renderer::VulkanRenderer::GetCurrentlyBoundType());
   renderData.viewMat = m_camera.GetViewMatrix();
   renderData.projMat = m_camera.GetProjectionMatrix();
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
      }
   }
   else
   {
      if (m_frameCount == NUM_FRAMES_TO_SAVE - 1)
      {
         // do nothing
      }
      else
      {
         ++m_frameCount;
      }
 
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
      vkCmdDrawIndexed(cmdBuffer, renderData.numMeshes.at(idx) * renderer::INDICES_PER_SPRITE, 1,
                       0, 0, 0);
   }

}

} // namespace looper
