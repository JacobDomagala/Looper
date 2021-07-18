#include "Game.hpp"
#include "Enemy.hpp"
#include "FileManager.hpp"
#include "RenderCommand.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

#include <GLFW/glfw3.h>
#include <fstream>
#include <string>

namespace dgame {

void
Game::MainLoop()
{
   Logger::SetLogType(Logger::TYPE::DEBUG);

   auto singleFrameTimer = 0.0f;

   while (IsRunning())
   {
      m_timer.ToggleTimer();
      singleFrameTimer += m_timer.GetFloatDeltaTime();

      while (IsRunning() && (singleFrameTimer > TARGET_TIME))
      {
         m_window->Clear();
         Renderer::BeginScene(m_camera);
         const auto dt = Timer::milliseconds(
            static_cast< long >(TARGET_TIME * 1000 * Timer::AreTimersRunning()));
         ProcessInput(dt);

         Render();
         if (m_frameTimer > 1.0f)
         {
            m_framesLastSecond = m_frames;
            m_frameTimer = 0.0f;
            m_frames = 0;
         }
         RenderText(std::to_string(m_framesLastSecond) + " FPS",
                    glm::vec2(-WIDTH / 2.0f, -HEIGHT / 2.0f), 0.4f, glm::vec3(1.0f, 0.0f, 1.0f));

         ++m_frames;
         m_frameTimer += singleFrameTimer;
         // singleFrameTimer = 0.0f;
         Renderer::EndScene();
         SwapBuffers();

         singleFrameTimer -= TARGET_TIME;

         InputManager::PollEvents();
      }
   }
}

void
Game::Init(const std::string configFile)
{
   m_logger.Init("Game");
   m_isGame = true;

   std::ifstream initFile((ASSETS_DIR / configFile).string());

   if (!initFile)
   {
      m_logger.Log(Logger::TYPE::FATAL, "Can't open" + (ASSETS_DIR / configFile).string());
   }

   m_window = std::make_unique< Window >(WIDTH, HEIGHT, "WindowTitle");

   RenderCommand::Init();
   RenderCommand::SetClearColor({1.0f, 0.2f, 0.3f, 1.0f});
   Renderer::Init();
   // RenderCommand::SetViewport(0, 0, WIDTH, HEIGHT);

   // m_frameBuffer.SetUp();

   while (!initFile.eof())
   {
      std::string tmp = "";
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

   // LoadLevel(m_levels[0]);
   m_state = GameState::GAME;
   m_initialized = true;
}

glm::ivec2
Game::CorrectPosition()
{
   glm::ivec2 playerPosition = m_playerPosition;
   glm::ivec2 levelSize = m_currentLevel->GetSize();

   auto linearPosition =
      static_cast< uint32_t >(floor(playerPosition.x + playerPosition.y * levelSize.x));

   byte_vec4* tmpCollision = m_collision;

   glm::ivec2 playerDestination = playerPosition;
   glm::ivec2 returnVal = glm::ivec2();

   if (playerPosition.x < 0)
   {
      returnVal.x = -playerPosition.x;
      return returnVal;
   }
   if (playerPosition.x >= levelSize.x)
   {
      returnVal.x = (levelSize.x - 1) - playerPosition.x;
      return returnVal;
   }
   if (playerPosition.y < 0)
   {
      returnVal.y = -playerPosition.y;
      return returnVal;
   }
   if (playerPosition.y >= levelSize.y)
   {
      returnVal.y = (levelSize.y - 1) - playerPosition.y;
      return returnVal;
   }
   // if you stand where you're not supposed to due to fucking float -> int32_t conversion
   // (FeelsBadMan)
   if (tmpCollision[linearPosition].w != 0)
   {
      // the value of getting you out of shit < -6 ; 5 >
      int32_t tmpval = 6;
      for (int32_t j = -tmpval; j <= tmpval; ++j)
      {
         for (int32_t i = -tmpval; i <= tmpval; ++i)
         {
            playerDestination = playerPosition + glm::ivec2(i, j);

            linearPosition = static_cast< uint32_t >(
               floor(playerDestination.x + playerDestination.y * levelSize.x));

            if ((playerDestination.x > 0) && (playerDestination.x < levelSize.x)
                && (playerDestination.y > 0) && (playerDestination.y < levelSize.y)
                && (tmpCollision[linearPosition].w == 0))
            {
               return glm::ivec2(playerDestination.x, playerDestination.y) - playerPosition;
            }
         }
      }
   }

   return glm::ivec2(0, 0);
}

std::pair< glm::ivec2, bool >
Game::CheckBulletCollision(Enemy* from, glm::vec2 globalTo, int32_t range)
{
   glm::ivec2 targetPixels = GlobalToScreen(globalTo);

   auto x1 = from->GetScreenPositionPixels().x;
   auto y1 = from->GetScreenPositionPixels().y;
   auto x2 = static_cast< float >(targetPixels.x);
   auto y2 = static_cast< float >(targetPixels.y);

   bool wasGreater = false;

   const bool steep = (glm::abs(y2 - y1) > glm::abs(x2 - x1));
   if (steep)
   {
      std::swap(x1, y1);
      std::swap(x2, y2);
   }

   if (x1 > x2)
   {
      wasGreater = true;
      std::swap(x1, x2);
      std::swap(y1, y2);
   }

   const float dx = x2 - x1;
   const float dy = glm::abs(y2 - y1);

   float error = dx / 2.0f;
   const int32_t ystep = (y1 < y2) ? 1 : -1;
   int32_t y = static_cast< int32_t >(y1);

   glm::ivec2 levelSize = m_currentLevel->GetSize();

   // Temporal fix to prevent warnings
   // Collision logic has to be rewritten anyway
   auto const int_x1 = static_cast< int32_t >(x1);
   auto const int_y1 = static_cast< int32_t >(y1);

   for (int32_t x = int_x1; x < int_x1 + range; x++)
   {
      if (steep)
      {
         // y,x
         glm::ivec2 tmpPos = glm::ivec2();
         if (!wasGreater)
            tmpPos = from->GetCenteredLocalPosition() + glm::ivec2(y - int_y1, x - int_x1);
         else
            tmpPos = from->GetCenteredLocalPosition() - glm::ivec2(y - int_y1, x - int_x1);

         if (!m_player->CheckCollision(tmpPos, from))
         {
            return {tmpPos, true};
         }

         if (tmpPos.x <= 0 || tmpPos.x >= levelSize.x || tmpPos.y <= 0 || tmpPos.y >= levelSize.y
             || m_collision[tmpPos.x + tmpPos.y * levelSize.x].w != 0)
         {
            return {tmpPos, false};
         }
      }
      else
      {
         // x,y
         glm::ivec2 tmpPos = glm::ivec2();
         if (!wasGreater)
         {
            tmpPos = from->GetCenteredLocalPosition() + glm::ivec2(x - int_x1, y - int_y1);
         }
         else
         {
            tmpPos = from->GetCenteredLocalPosition() - glm::ivec2(x - int_x1, y - int_y1);
         }

         if (!m_player->CheckCollision(tmpPos, from))
         {
            return {tmpPos, true};
         }

         if (tmpPos.x <= 0 || tmpPos.x >= levelSize.x || tmpPos.y <= 0 || tmpPos.y >= levelSize.y
             || m_collision[tmpPos.x + tmpPos.y * levelSize.x].w != 0)
         {
            return {tmpPos, false};
         }
      }

      error -= dy;
      if (error < 0)
      {
         y += ystep;
         error += dx;
      }
   }
   return {glm::ivec2(), false};
}

bool
Game::IsPlayerInVision(Enemy* from, int32_t range)
{
   glm::ivec2 targetPixels = GlobalToScreen(m_player->GetCenteredGlobalPosition());

   float x1 = from->GetScreenPositionPixels().x;
   float y1 = from->GetScreenPositionPixels().y;
   float x2 = static_cast< float >(targetPixels.x);
   float y2 = static_cast< float >(targetPixels.y);

   bool wasGreater = false;
   const bool steep = (glm::abs(y2 - y1) > glm::abs(x2 - x1));
   if (steep)
   {
      std::swap(x1, y1);
      std::swap(x2, y2);
   }

   if (x1 > x2)
   {
      wasGreater = true;
      std::swap(x1, x2);
      std::swap(y1, y2);
   }

   const float dx = x2 - x1;
   const float dy = glm::abs(y2 - y1);

   float error = dx / 2.0f;
   const int32_t ystep = (y1 < y2) ? 1 : -1;
   int32_t y = static_cast< int32_t >(y1);

   glm::ivec2 levelSize = m_currentLevel->GetSize();

   // Temporal fix to prevent warnings
   // Collision logic has to be rewritten anyway
   auto const int_x1 = static_cast< int32_t >(x1);
   auto const int_y1 = static_cast< int32_t >(y1);

   for (int32_t x = int_x1; x < int_x1 + range; x++)
   {
      if (steep)
      {
         // y,x
         glm::ivec2 tmpPos = glm::ivec2();
         if (!wasGreater)
            tmpPos = from->GetCenteredLocalPosition() + glm::ivec2(y - int_y1, x - int_x1);
         else
            tmpPos = from->GetCenteredLocalPosition() - glm::ivec2(y - int_y1, x - int_x1);

         if (!m_player->CheckCollision(tmpPos, from, false))
         {
            return true;
         }

         if (tmpPos.x <= 0 || tmpPos.x >= levelSize.x || tmpPos.y <= 0 || tmpPos.y >= levelSize.y
             || m_collision[tmpPos.x + tmpPos.y * levelSize.x].w != 0)
         {
            return false;
         }
      }
      else
      {
         // x,y
         glm::ivec2 tmpPos = glm::ivec2();
         if (!wasGreater)
         {
            tmpPos = from->GetCenteredLocalPosition() + glm::ivec2(x - int_x1, y - int_y1);
         }
         else
         {
            tmpPos = from->GetCenteredLocalPosition() - glm::ivec2(x - int_x1, y - int_y1);
         }

         if (!m_player->CheckCollision(tmpPos, from, false))
         {
            return true;
         }

         if (tmpPos.x <= 0 || tmpPos.x >= levelSize.x || tmpPos.y <= 0 || tmpPos.y >= levelSize.y
             || m_collision[tmpPos.x + tmpPos.y * levelSize.x].w != 0)
         {
            return false;
         }
      }

      error -= dy;
      if (error < 0)
      {
         y += ystep;
         error += dx;
      }
   }
   return false;
}

glm::ivec2
Game::CheckBulletCollision(int32_t range)
{
   float x1, x2, y1, y2;
   x1 = static_cast< float >(m_player->GetScreenPositionPixels().x);
   y1 = static_cast< float >(m_player->GetScreenPositionPixels().y);
   x2 = m_window->GetCursor().x;
   y2 = m_window->GetCursor().y;

   bool wasGreater = false;
   const bool steep = (glm::abs(y2 - y1) > glm::abs(x2 - x1));
   if (steep)
   {
      std::swap(x1, y1);
      std::swap(x2, y2);
   }

   if (x1 > x2)
   {
      wasGreater = true;
      std::swap(x1, x2);
      std::swap(y1, y2);
   }

   const float dx = x2 - x1;
   const float dy = glm::abs(y2 - y1);

   float error = dx / 2.0f;
   const int32_t ystep = (y1 < y2) ? 1 : -1;
   int32_t y = static_cast< int32_t >(y1);

   const auto maxX = static_cast< int32_t >(x2);

   glm::ivec2 levelSize = m_currentLevel->GetSize();

   // Temporal fix to prevent warnings
   // Collision logic has to be rewritten anyway
   auto const int_x1 = static_cast< int32_t >(x1);
   auto const int_y1 = static_cast< int32_t >(y1);

   for (auto x = int_x1; x < maxX + range; x++)
   {
      if (steep)
      {
         // y,x
         glm::ivec2 tmpPos = glm::ivec2();
         if (!wasGreater)
         {
            tmpPos = m_player->GetCenteredLocalPosition() + glm::ivec2(y - int_y1, x - int_x1);
         }
         else
         {
            tmpPos = m_player->GetCenteredLocalPosition() - glm::ivec2(y - int_y1, x - int_x1);
         }

         if (!m_currentLevel->CheckCollision(tmpPos, *m_player))
         {
            return tmpPos;
         }

         if (tmpPos.x == 0 || tmpPos.x == levelSize.x || tmpPos.y == 0 || tmpPos.y == levelSize.y
             || m_collision[tmpPos.x + tmpPos.y * levelSize.x].w != 0)
         {
            return tmpPos;
         }
      }
      else
      {
         // x,y
         glm::ivec2 tmpPos = glm::ivec2();
         if (!wasGreater)
         {
            tmpPos = m_player->GetCenteredLocalPosition() + glm::ivec2(x - int_x1, y - int_y1);
         }
         else
         {
            tmpPos = m_player->GetCenteredLocalPosition() - glm::ivec2(x - int_x1, y - int_y1);
         }

         if (!m_currentLevel->CheckCollision(tmpPos, *m_player))
         {
            return tmpPos;
         }

         if (tmpPos.x == 0 || tmpPos.x == levelSize.x || tmpPos.y == 0 || tmpPos.y == levelSize.y
             || m_collision[tmpPos.x + tmpPos.y * levelSize.x].w != 0)
         {
            return tmpPos;
         }
      }

      error -= dy;
      if (error < 0)
      {
         y += ystep;
         error += dx;
      }
   }
   return glm::ivec2();
}

glm::ivec2
Game::CheckCollision(glm::ivec2& moveBy)
{
   glm::ivec2 levelSize = m_currentLevel->GetSize();
   glm::ivec2 playerPosition = m_player->GetLocalPosition(); // m_playerPosition;

   glm::ivec2 destination = playerPosition + moveBy;
   glm::ivec2 returnVal = glm::ivec2();

   if (destination.x < 0 || destination.x > levelSize.x || destination.y < 0
       || destination.y > levelSize.y)
   {
      return returnVal;
   }

   int32_t distance = static_cast< int32_t >(glm::length(static_cast< glm::vec2 >(moveBy)));

   glm::vec2 nMoveBy = glm::normalize(glm::vec2(moveBy));
   glm::ivec2 direction = glm::ivec2(glm::ceil(nMoveBy.x), glm::ceil(nMoveBy.y));

   uint32_t linearDestination =
      static_cast< uint32_t >(floor(destination.x + destination.y * levelSize.x));
   uint32_t linearPosition =
      static_cast< uint32_t >(floor(playerPosition.x + playerPosition.y * levelSize.x));
   auto linearLevelSize = levelSize.x * levelSize.y;

   uint32_t tmpPosition = linearPosition;
   returnVal = moveBy;
   byte_vec4* tmpCollision = m_collision;

   // if player can move to destination (FeelsGoodMan)
   if (tmpCollision[linearDestination].w == 0)
   {
      return returnVal;
   }

   // if player is standing in shit, this will take it into account
   glm::ivec2 positionBias = glm::ivec2();

   // if player couldn't get to destination, find closest in line point where you can go
   for (int32_t i = distance; i > 0; --i)
   {
      glm::ivec2 tmpDest = playerPosition + direction * i;
      tmpPosition = static_cast< uint32_t >(floor(tmpDest.x + tmpDest.y * levelSize.x));
      if ((tmpPosition > 0) && (tmpPosition < static_cast< uint32_t >(linearLevelSize))
          && (tmpDest.x < levelSize.x) && (tmpDest.y < levelSize.y)
          && (tmpCollision[tmpPosition].w == 0))
      {
         returnVal = tmpDest - playerPosition;
         returnVal += positionBias;
         return returnVal;
      }
   }

   // if player couldn't find anything in straight line, check the closest boundaries
   for (int32_t j = distance; j > 0; --j)
   {
      if (direction.x == 0)
      {
         for (int32_t i = -2; i <= 2; ++i)
         {
            glm::ivec2 tmpDirection = glm::ivec2(direction.x + i, direction.y);
            glm::ivec2 tmpDest = playerPosition + tmpDirection * j;

            tmpPosition = static_cast< uint32_t >(floor(tmpDest.x + tmpDest.y * levelSize.x));
            if ((tmpDest.x > 0) && (tmpDest.y > 0) && (tmpDest.x < levelSize.x)
                && (tmpDest.y < levelSize.y) && (tmpCollision[tmpPosition].w == 0))
            {
               returnVal = tmpDest - playerPosition;
               returnVal += positionBias;
               return returnVal;
            }
         }
      }
      else if (direction.y == 0)
      {
         for (int32_t i = -2; i <= 2; ++i)
         {
            glm::ivec2 tmpDirection = glm::ivec2(direction.x, direction.y + i);
            glm::ivec2 tmpDest = playerPosition + tmpDirection * j;

            tmpPosition = static_cast< uint32_t >(floor(tmpDest.x + tmpDest.y * levelSize.x));
            if ((tmpDest.x > 0) && (tmpDest.y > 0) && (tmpDest.x < levelSize.x)
                && (tmpDest.y < levelSize.y) && (tmpCollision[tmpPosition].w == 0))
            {
               returnVal = tmpDest - playerPosition;
               returnVal += positionBias;
               return returnVal;
            }
         }
      }
   }

   // worst case scenario player won't move
   return glm::ivec2() + positionBias;
}

glm::ivec2
Game::CheckCollision(const glm::ivec2& currentPosition, const glm::ivec2& moveBy)
{
   glm::ivec2 levelSize = m_currentLevel->GetSize();
   glm::ivec2 playerPosition = currentPosition;

   glm::ivec2 destination = playerPosition + moveBy;
   glm::ivec2 returnVal = glm::ivec2();

   if (destination.x < 0 || destination.x > levelSize.x || destination.y < 0
       || destination.y > levelSize.y)
   {
      return returnVal;
   }

   int32_t distance = static_cast< int32_t >(glm::length(static_cast< glm::vec2 >(moveBy)));

   glm::vec2 nMoveBy = glm::normalize(glm::vec2(moveBy));
   glm::ivec2 direction = glm::ivec2(glm::ceil(nMoveBy.x), glm::ceil(nMoveBy.y));

   uint32_t linearDestination =
      static_cast< uint32_t >(floor(destination.x + destination.y * levelSize.x));
   uint32_t linearPosition =
      static_cast< uint32_t >(floor(playerPosition.x + playerPosition.y * levelSize.x));
   auto linearLevelSize = levelSize.x * levelSize.y;

   uint32_t tmpPosition = linearPosition;
   returnVal = moveBy;
   byte_vec4* tmpCollision = m_collision;

   // if player can move to destination (FeelsGoodMan)
   if (tmpCollision[linearDestination].w == 0)
   {
      return returnVal;
   }

   // if player is standing in shit, this will take it into account
   glm::ivec2 positionBias = glm::ivec2();

   // if player couldn't get to destination, find closest in line point where you can go
   for (int32_t i = distance; i > 0; --i)
   {
      glm::ivec2 tmpDest = playerPosition + direction * i;
      tmpPosition = static_cast< uint32_t >(floor(tmpDest.x + tmpDest.y * levelSize.x));
      if ((tmpPosition > 0) && (tmpPosition < static_cast< uint32_t >(linearLevelSize))
          && (tmpDest.x < levelSize.x) && (tmpDest.y < levelSize.y)
          && (tmpCollision[tmpPosition].w == 0))
      {
         returnVal = tmpDest - playerPosition;
         returnVal += positionBias;
         return returnVal;
      }
   }

   // if player couldn't find anything in straight line, check the closest boundaries
   for (int32_t j = distance; j > 0; --j)
   {
      if (direction.x == 0)
      {
         for (int32_t i = -2; i <= 2; ++i)
         {
            glm::ivec2 tmpDirection = glm::ivec2(direction.x + i, direction.y);
            glm::ivec2 tmpDest = playerPosition + tmpDirection * j;

            tmpPosition = static_cast< uint32_t >(floor(tmpDest.x + tmpDest.y * levelSize.x));
            if ((tmpDest.x > 0) && (tmpDest.y > 0) && (tmpDest.x < levelSize.x)
                && (tmpDest.y < levelSize.y) && (tmpCollision[tmpPosition].w == 0))
            {
               returnVal = tmpDest - playerPosition;
               returnVal += positionBias;
               return returnVal;
            }
         }
      }
      else if (direction.y == 0)
      {
         for (int32_t i = -2; i <= 2; ++i)
         {
            glm::ivec2 tmpDirection = glm::ivec2(direction.x, direction.y + i);
            glm::ivec2 tmpDest = playerPosition + tmpDirection * j;

            tmpPosition = static_cast< uint32_t >(floor(tmpDest.x + tmpDest.y * levelSize.x));
            if ((tmpDest.x > 0) && (tmpDest.y > 0) && (tmpDest.x < levelSize.x)
                && (tmpDest.y < levelSize.y) && (tmpCollision[tmpPosition].w == 0))
            {
               returnVal = tmpDest - playerPosition;
               returnVal += positionBias;
               return returnVal;
            }
         }
      }
   }

   // worst case scenario player won't move
   return positionBias;
}

bool
Game::CheckMove(glm::vec2& moveBy)
{
   // moveBy = CheckCollision(moveBy);
   // m_player->Move(moveBy, false);

   return glm::length(glm::vec2(moveBy)) > 0;
}

void
Game::KeyEvents()
{
   // Camera movement is disabled
   int32_t cameraMovement =
      static_cast< int32_t >(0.0f * static_cast< float >(m_deltaTime.count()));
   int32_t playerMovement =
      static_cast< int32_t >(0.5f * static_cast< float >(m_deltaTime.count()));

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
         glEnable(GL_BLEND);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
         glDisable(GL_BLEND);
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_W))
      {
         playerMoveBy += glm::vec2(0, -playerMovement);
         cameraMoveBy += glm::vec2(0, cameraMovement);
      }
      if (InputManager::CheckKeyPressed(GLFW_KEY_S))
      {
         playerMoveBy += glm::vec2(0, playerMovement);
         cameraMoveBy += glm::vec2(0, -cameraMovement);
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

      if (glm::length(glm::vec2(playerMoveBy)) > 0.0f)
      {
         //m_currentLevel->Move(cameraMoveBy);
         m_camera.Move(glm ::vec3{cameraMoveBy, 0.0f});
         m_player->Move(playerMoveBy);

         //if (CheckMove(playerMoveBy) == false)
         //{
         //   //m_currentLevel->Move(-cameraMoveBy);
         //}
      }
   }
}

void
Game::MouseEvents()
{
   glm::vec2 tmp = CheckBulletCollision(m_player->GetWeaponRange());

   /*DrawLine(m_currentLevel->GetGlobalVec(m_player->GetCenteredLocalPosition()),
      m_currentLevel->GetGlobalVec(tmp), glm::vec3(0.0f, 1.0f, 0.0f));*/
   Renderer::DrawLine(m_player->GetLocalPosition(), tmp, {0.8f, 0.0f, 0.3f, 1.0f});
   ////PRIMARY FIRE
   // if (Win_Window::GetKeyState(VK_LBUTTON))
   //{
   //	timer.ToggleTimer();
   //
   //	delta += timer.GetDeltaTime();
   //	if (delta >= player.GetReloadTime())
   //		primaryFire = false;

   //	if (!primaryFire)
   //	{
   //		player.Shoot();
   //		glm::vec2 tmp = CheckBulletCollision(player.GetWeaponRange());
   //		//DrawLine(player.GetGlobalPosition(), currentLevel.GetGlobalVec(tmp));
   //		primaryFire = true;
   //		delta = 0.0f;
   //	}
   //}

   ////ALTERNATIVE FIRE
   //	if (Win_Window::GetKeyState(VK_RBUTTON))
   //{
   //	alternativeFire = true;
   //	player.Shoot();
   //}

   // TODO: Find some easier formula for this?
   if (!m_reverse)
   {
      // value to control how fast should camera move
      int32_t multiplier = 3;

      // cursor's position from center of the screen to trigger camera movement
      float borderValue = 0.5f;

      float cameraMovement = floorf(static_cast< float >(m_deltaTime.count()));
      auto cameraMoveBy = glm::ivec2();
      const auto cursor = m_window->GetCursorNormalized();

      if (cursor.x > borderValue)
      {
         float someX = (cursor.x - borderValue) * static_cast< float >(multiplier);
         cameraMoveBy += glm::vec2(cameraMovement * someX, 0.0f);
      }
      else if (cursor.x < -borderValue)
      {
         float someX = (cursor.x + borderValue) * static_cast< float >(multiplier);
         cameraMoveBy += glm::vec2(cameraMovement * someX, 0.0f);
      }
      if (cursor.y > borderValue)
      {
         float someY = (cursor.y - borderValue) * static_cast< float >(multiplier);
         cameraMoveBy += glm::vec2(0.0f, cameraMovement * someY);
      }
      else if (cursor.y < -borderValue)
      {
         float someY = (cursor.y + borderValue) * static_cast< float >(multiplier);
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

   // Temporary fix for getting player unstuck from walls
   // Should only be active when not going in reverse mode
   // if (!m_reverse)
   //{
   //   // player's position on the map
   //   m_playerPosition = m_currentLevel->GetLocalVec(m_player->GetCenteredGlobalPosition());

   //   glm::ivec2 correction = CorrectPosition();

   //   m_player->Move(correction);
   //   m_playerPosition += correction;
   //   m_player->SetCenteredLocalPosition(m_playerPosition);
   //}

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
   m_currentLevel = std::make_shared< Level >();
   m_currentLevel->Load(this, pathToLevel);
   m_player = m_currentLevel->GetPlayer();

   m_camera.Create(glm::vec3(m_player->GetCenteredGlobalPosition(), 0.0f), m_window->GetSize());
   m_camera.SetLevelSize(m_currentLevel->GetSize());
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
Game::GetZoomLevel()
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

void
Game::SwapBuffers()
{
   m_window->SwapBuffers();
}

bool
Game::IsRunning()
{
   return m_window->IsRunning();
}

void
Game::RegisterForKeyInput(InputListener* listener)
{
   InputManager::RegisterForKeyInput(listener);
}

void
Game::SetCollisionMap(byte_vec4* collision)
{
   m_collision = collision;
}

void
Game::ProcessInput(Timer::milliseconds deltaTime)
{
   m_deltaTime = deltaTime;

   MouseEvents();
   KeyEvents();

   HandleReverseLogic();
   UpdateGameState();
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
Game::Render()
{
   RenderFirstPass();
   RenderSecondPass();
}

} // namespace dgame
