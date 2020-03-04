#include "Game.hpp"
#include "Enemy.hpp"
#include "FileManager.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <GL/glew.h>
#include <fstream>
#include <stb_image.h>
#include <string>

// glm::vec2 destination;
glm::vec2 cursor;
glm::vec2 debug1;

static float delta = 0.0f;

glm::ivec2
Game::GlobalToScreen(glm::vec2 globalPos)
{
   // convert to <-1, 1>
   auto screenPosition = m_window->GetProjection() * glm::vec4(globalPos, 0.0f, 1.0f);

   // convert to <0, 1>
   auto returnPos = (glm::vec2(screenPosition.x, screenPosition.y) + glm::vec2(1.0f, 1.0f)) / glm::vec2(2.0f, 2.0f);

   // convert to <0, WIDTH>, <0, HEIGHT>
   // with y = 0 in top left corner
   returnPos.x *= WIDTH;
   returnPos.y *= -HEIGHT;
   returnPos.y += HEIGHT;

   return returnPos;
}

void
Game::Init(const std::string configFile)
{
   std::ifstream initFile((ASSETS_DIR / configFile).u8string());

   if (!initFile)
   {
      logger.Log(Logger::TYPE::FATAL, "Can't open" + (ASSETS_DIR / configFile).u8string());
   }

   m_cameraSpeed = 600.0f;
   m_window = std::make_unique< Window >(WIDTH, HEIGHT, "WindowTitle");
   m_frameBuffer.SetUp();

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
         m_font.SetFont(tmp);
      }
   }

   initFile.close();

   m_inputManager.Init(m_window->GetWindowHandle());

   LoadLevel(m_levels[0]);
   m_state = GameState::GAME;
}

glm::ivec2
Game::CorrectPosition()
{
   glm::ivec2 playerPosition = m_playerPosition;
   glm::ivec2 levelSize = m_currentLevel.GetSize();

   auto linearPosition = static_cast< uint32_t >(floor(playerPosition.x + playerPosition.y * levelSize.x));

   byte_vec4* tmpCollision = m_collision.get();

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
   // if you stand where you're not supposed to due to fucking float -> int32_t conversion (FeelsBadMan)
   if (tmpCollision[linearPosition].w != 0)
   {
      // the value of getting you out of shit < -6 ; 5 >
      int32_t tmpval = 6;
      for (int32_t j = -tmpval; j <= tmpval; ++j)
      {
         for (int32_t i = -tmpval; i <= tmpval; ++i)
         {
            playerDestination = playerPosition + glm::ivec2(i, j);

            linearPosition = static_cast< uint32_t >(floor(playerDestination.x + playerDestination.y * levelSize.x));

            if ((playerDestination.x > 0) && (playerDestination.x < levelSize.x) && (playerDestination.y > 0)
                && (playerDestination.y < levelSize.y) && (tmpCollision[linearPosition].w == 0))
            {
               return glm::ivec2(playerDestination.x, playerDestination.y) - playerPosition;
            }
         }
      }
   }

   return glm::ivec2(0, 0);
}

void
Game::DrawLine(glm::vec2 from, glm::vec2 to, glm::vec3 color)
{
   m_debugObjs.push_back(std::make_unique< Line >(from, to, color));
}

void
Game::RenderLine(const glm::ivec2& collided, const glm::vec3& color)
{
   glm::vec2 lineCollided = m_currentLevel.GetGlobalVec(collided);

   Shaders lineShader;
   lineShader.LoadShaders("../lineVertex.glsl", "../lineFragment.glsl");

   glm::vec2 vertices[2] = {glm::vec2(m_player->GetCenteredGlobalPosition()), glm::vec2(lineCollided)};
   glm::mat4 modelMatrix = glm::scale(glm::mat4(), glm::vec3(1.0f, 1.0f, 1.0f));
   GLuint lineVertexArray;
   GLuint lineVertexBuffer;
   glGenVertexArrays(1, &lineVertexArray);
   glGenBuffers(1, &lineVertexBuffer);
   glBindVertexArray(lineVertexArray);
   glBindBuffer(GL_ARRAY_BUFFER, lineVertexBuffer);
   glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 2, vertices, GL_DYNAMIC_DRAW);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

   lineShader.UseProgram();
   lineShader.SetUniformFloatMat4(modelMatrix, "modelMatrix");
   lineShader.SetUniformFloatMat4(m_window->GetProjection(), "projectionMatrix");
   lineShader.SetUniformFloatVec4(glm::vec4(color, 1.0f), "color");

   glDrawArrays(GL_LINES, 0, 2);
   glBindVertexArray(0);
   glDeleteBuffers(1, &lineVertexBuffer);
   glDeleteVertexArrays(1, &lineVertexArray);
}

std::pair< glm::ivec2, bool >
Game::CheckBulletCollision(Enemy* from, glm::vec2 globalTo, int32_t range)
{
   glm::ivec2 targetPixels = GlobalToScreen(globalTo);

   float x1 = from->GetScreenPositionPixels().x;
   float y1 = from->GetScreenPositionPixels().y;
   float x2 = static_cast< float >(targetPixels.x);
   float y2 = static_cast< float >(targetPixels.y);

   bool wasGreater = false;
   const bool steep = (fabs(y2 - y1) > fabs(x2 - x1));
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
   const float dy = fabs(y2 - y1);

   float error = dx / 2.0f;
   const int32_t ystep = (y1 < y2) ? 1 : -1;
   int32_t y = static_cast< int32_t >(y1);

   const int32_t maxX = static_cast< int32_t >(x2);

   glm::ivec2 levelSize = m_currentLevel.GetSize();

   for (int32_t x = static_cast< int32_t >(x1); x < x1 + range; x++)
   {
      if (steep)
      {
         // y,x
         glm::ivec2 tmpPos = glm::ivec2();
         if (!wasGreater)
            tmpPos = from->GetCenteredLocalPosition() + glm::ivec2(y - y1, x - x1);
         else
            tmpPos = from->GetCenteredLocalPosition() - glm::ivec2(y - y1, x - x1);

         if (!m_player->CheckCollision(tmpPos, from))
         {
            return {tmpPos, true};
         }

         if (tmpPos.x <= 0 || tmpPos.x >= levelSize.x || tmpPos.y <= 0 || tmpPos.y >= levelSize.y
             || m_collision.get()[tmpPos.x + tmpPos.y * levelSize.x].w != 0)
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
            tmpPos = from->GetCenteredLocalPosition() + glm::ivec2(x - x1, y - y1);
         }
         else
         {
            tmpPos = from->GetCenteredLocalPosition() - glm::ivec2(x - x1, y - y1);
         }

         if (!m_player->CheckCollision(tmpPos, from))
         {
            return {tmpPos, true};
         }

         if (tmpPos.x <= 0 || tmpPos.x >= levelSize.x || tmpPos.y <= 0 || tmpPos.y >= levelSize.y
             || m_collision.get()[tmpPos.x + tmpPos.y * levelSize.x].w != 0)
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
   const bool steep = (fabs(y2 - y1) > fabs(x2 - x1));
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
   const float dy = fabs(y2 - y1);

   float error = dx / 2.0f;
   const int32_t ystep = (y1 < y2) ? 1 : -1;
   int32_t y = static_cast< int32_t >(y1);

   const int32_t maxX = static_cast< int32_t >(x2);

   glm::ivec2 levelSize = m_currentLevel.GetSize();

   for (int32_t x = static_cast< int32_t >(x1); x < x1 + range; x++)
   {
      if (steep)
      {
         // y,x
         glm::ivec2 tmpPos = glm::ivec2();
         if (!wasGreater)
            tmpPos = from->GetCenteredLocalPosition() + glm::ivec2(y - y1, x - x1);
         else
            tmpPos = from->GetCenteredLocalPosition() - glm::ivec2(y - y1, x - x1);

         if (!m_player->CheckCollision(tmpPos, from, false))
         {
            return true;
         }

         if (tmpPos.x <= 0 || tmpPos.x >= levelSize.x || tmpPos.y <= 0 || tmpPos.y >= levelSize.y
             || m_collision.get()[tmpPos.x + tmpPos.y * levelSize.x].w != 0)
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
            tmpPos = from->GetCenteredLocalPosition() + glm::ivec2(x - x1, y - y1);
         }
         else
         {
            tmpPos = from->GetCenteredLocalPosition() - glm::ivec2(x - x1, y - y1);
         }

         if (!m_player->CheckCollision(tmpPos, from, false))
         {
            return true;
         }

         if (tmpPos.x <= 0 || tmpPos.x >= levelSize.x || tmpPos.y <= 0 || tmpPos.y >= levelSize.y
             || m_collision.get()[tmpPos.x + tmpPos.y * levelSize.x].w != 0)
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
   const bool steep = (fabs(y2 - y1) > fabs(x2 - x1));
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
   const float dy = fabs(y2 - y1);

   float error = dx / 2.0f;
   const int32_t ystep = (y1 < y2) ? 1 : -1;
   int32_t y = static_cast< int32_t >(y1);

   const int32_t maxX = (int32_t)x2;

   glm::ivec2 levelSize = m_currentLevel.GetSize();

   for (int32_t x = static_cast< int32_t >(x1); x < maxX + range; x++)
   {
      if (steep)
      {
         // y,x
         glm::ivec2 tmpPos = glm::ivec2();
         if (!wasGreater)
         {
            tmpPos = m_player->GetCenteredLocalPosition() + glm::ivec2(y - y1, x - x1);
         }
         else
         {
            tmpPos = m_player->GetCenteredLocalPosition() - glm::ivec2(y - y1, x - x1);
         }

         if (!m_currentLevel.CheckCollision(tmpPos, *m_player))
         {
            return tmpPos;
         }

         if (tmpPos.x == 0 || tmpPos.x == levelSize.x || tmpPos.y == 0 || tmpPos.y == levelSize.y
             || m_collision.get()[tmpPos.x + tmpPos.y * levelSize.x].w != 0)
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
            tmpPos = m_player->GetCenteredLocalPosition() + glm::ivec2(x - x1, y - y1);
         }
         else
         {
            tmpPos = m_player->GetCenteredLocalPosition() - glm::ivec2(x - x1, y - y1);
         }

         if (!m_currentLevel.CheckCollision(tmpPos, *m_player))
         {
            return tmpPos;
         }

         if (tmpPos.x == 0 || tmpPos.x == levelSize.x || tmpPos.y == 0 || tmpPos.y == levelSize.y
             || m_collision.get()[tmpPos.x + tmpPos.y * levelSize.x].w != 0)
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
   glm::ivec2 levelSize = m_currentLevel.GetSize();
   glm::ivec2 playerPosition = m_playerPosition;

   glm::ivec2 destination = playerPosition + moveBy;
   glm::ivec2 returnVal = glm::ivec2();

   if (destination.x < 0 || destination.x > levelSize.x || destination.y < 0 || destination.y > levelSize.y)
   {
      return returnVal;
   }

   int32_t distance = static_cast< int32_t >(glm::length(static_cast< glm::vec2 >(moveBy)));

   glm::vec2 nMoveBy = glm::normalize(glm::vec2(moveBy));
   glm::ivec2 direction = glm::ivec2(ceil(nMoveBy.x), ceil(nMoveBy.y));

   uint32_t linearDestination = static_cast< uint32_t >(floor(destination.x + destination.y * levelSize.x));
   uint32_t linearPosition = static_cast< uint32_t >(floor(playerPosition.x + playerPosition.y * levelSize.x));
   uint32_t linearLevelSize = levelSize.x * levelSize.y;

   uint32_t tmpPosition = linearPosition;
   returnVal = moveBy;
   byte_vec4* tmpCollision = m_collision.get();

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
      if ((tmpPosition > 0) && (tmpPosition < linearLevelSize) && (tmpDest.x < levelSize.x) && (tmpDest.y < levelSize.y)
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
            if ((tmpDest.x > 0) && (tmpDest.y > 0) && (tmpDest.x < levelSize.x) && (tmpDest.y < levelSize.y)
                && (tmpCollision[tmpPosition].w == 0))
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
            if ((tmpDest.x > 0) && (tmpDest.y > 0) && (tmpDest.x < levelSize.x) && (tmpDest.y < levelSize.y)
                && (tmpCollision[tmpPosition].w == 0))
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
   glm::ivec2 levelSize = m_currentLevel.GetSize();
   glm::ivec2 playerPosition = currentPosition;

   glm::ivec2 destination = playerPosition + moveBy;
   glm::ivec2 returnVal = glm::ivec2();

   if (destination.x < 0 || destination.x > levelSize.x || destination.y < 0 || destination.y > levelSize.y)
   {
      return returnVal;
   }

   int32_t distance = static_cast< int32_t >(glm::length(static_cast< glm::vec2 >(moveBy)));

   glm::vec2 nMoveBy = glm::normalize(glm::vec2(moveBy));
   glm::ivec2 direction = glm::ivec2(ceil(nMoveBy.x), ceil(nMoveBy.y));

   uint32_t linearDestination = static_cast< uint32_t >(floor(destination.x + destination.y * levelSize.x));
   uint32_t linearPosition = static_cast< uint32_t >(floor(playerPosition.x + playerPosition.y * levelSize.x));
   uint32_t linearLevelSize = levelSize.x * levelSize.y;

   uint32_t tmpPosition = linearPosition;
   returnVal = moveBy;
   byte_vec4* tmpCollision = m_collision.get();

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
      if ((tmpPosition > 0) && (tmpPosition < linearLevelSize) && (tmpDest.x < levelSize.x) && (tmpDest.y < levelSize.y)
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
            if ((tmpDest.x > 0) && (tmpDest.y > 0) && (tmpDest.x < levelSize.x) && (tmpDest.y < levelSize.y)
                && (tmpCollision[tmpPosition].w == 0))
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
            if ((tmpDest.x > 0) && (tmpDest.y > 0) && (tmpDest.x < levelSize.x) && (tmpDest.y < levelSize.y)
                && (tmpCollision[tmpPosition].w == 0))
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

bool
Game::CheckMove(glm::ivec2& moveBy)
{
   moveBy = CheckCollision(moveBy);
   m_player->Move(moveBy);

   if (glm::length(glm::vec2(moveBy)))
   {
      return true;
   }

   return false;
}

void
Game::KeyEvents(float deltaTime)
{
   int32_t cameraMovement = static_cast< int32_t >(300.0f * m_deltaTime);
   int32_t playerMovement = static_cast< int32_t >(500.0f * m_deltaTime);

   glm::ivec2 playerMoveBy = glm::ivec2();
   glm::ivec2 cameraMoveBy = glm::ivec2();

   if (m_inputManager.CheckKeyPressed(GLFW_KEY_UP))
   {
      m_reverse = true;
   }
   if (m_inputManager.CheckKeyPressed(GLFW_KEY_DOWN))
   {
      m_reverse = false;
   }

   if (!m_reverse)
   {
      if (m_inputManager.CheckKeyPressed(GLFW_KEY_LEFT))
      {
         m_currentLevel.MoveObjs(glm::vec2(2.0f, 0.0f));
      }
      if (m_inputManager.CheckKeyPressed(GLFW_KEY_RIGHT))
      {
         m_currentLevel.MoveObjs(glm::vec2(-2.0f, 0.0f));
      }
      if (m_inputManager.CheckKeyPressed(GLFW_KEY_ESCAPE))
      {
         m_window->ShutDown();
      }
      if (m_inputManager.CheckKeyPressed(GLFW_KEY_O))
      {
         glEnable(GL_BLEND);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      }
      if (m_inputManager.CheckKeyPressed(GLFW_KEY_1))
      {
         m_window->ShowCursor(true);
      }
      if (m_inputManager.CheckKeyPressed(GLFW_KEY_2))
      {
         m_window->ShowCursor(false);
      }
      if (m_inputManager.CheckKeyPressed(GLFW_KEY_P))
      {
         glDisable(GL_BLEND);
      }
      if (m_inputManager.CheckKeyPressed(GLFW_KEY_W))
      {
         playerMoveBy += glm::ivec2(0, -playerMovement);
         cameraMoveBy += glm::ivec2(0, cameraMovement);
      }
      if (m_inputManager.CheckKeyPressed(GLFW_KEY_S))
      {
         playerMoveBy += glm::ivec2(0, playerMovement);
         cameraMoveBy += glm::ivec2(0, -cameraMovement);
      }
      if (m_inputManager.CheckKeyPressed(GLFW_KEY_A))
      {
         playerMoveBy += glm::ivec2(-playerMovement, 0);
         cameraMoveBy += glm::ivec2(cameraMovement, 0);
      }
      if (m_inputManager.CheckKeyPressed(GLFW_KEY_D))
      {
         playerMoveBy += glm::ivec2(playerMovement, 0);
         cameraMoveBy += glm::ivec2(-cameraMovement, 0);
      }
      if (m_inputManager.CheckKeyPressed(GLFW_KEY_R))
      {
         Timer::PauseAllTimers();
      }
      if (m_inputManager.CheckKeyPressed(GLFW_KEY_T))
      {
         Timer::ResumeAllTimers();
      }
      if (m_inputManager.CheckKeyPressed(GLFW_KEY_SPACE))
      {
         m_currentLevel.Move(-m_player->GetCenteredGlobalPosition());
         m_player->Move(-m_player->GetCenteredGlobalPosition());
      }

      if (glm::length(glm::vec2(playerMoveBy)))
      {
         m_currentLevel.Move(cameraMoveBy);
         if (CheckMove(playerMoveBy) == false)
            m_currentLevel.Move(-cameraMoveBy);
      }
   }
}

void
Game::MouseEvents(float deltaTime)
{
   float cameraMovement = floor(m_cameraSpeed * m_deltaTime);
   glm::ivec2 cameraMoveBy = glm::ivec2();

   cursor = m_window->GetCursorNormalized();
   // glm::vec2 tmp = CheckBulletCollision(m_player->GetWeaponRange());

   // DrawLine(m_currentLevel.GetGlobalVec(m_player->GetCenteredLocalPosition()), m_currentLevel.GetGlobalVec(tmp), glm::vec3(0.0f, 1.0f,
   // 0.0f));

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

   // value to control how fast should camera move
   int32_t multiplier = 3;

   // cursor's position from center of the screen to trigger camera movement
   float borderValue = 0.2f;

   if (!m_reverse)
   {
      if (cursor.x > borderValue)
      {
         float someX = (cursor.x - borderValue) * multiplier;
         cameraMoveBy += glm::vec2(-cameraMovement * someX, 0.0f);
      }
      else if (cursor.x < -borderValue)
      {
         float someX = (cursor.x + borderValue) * multiplier;
         cameraMoveBy += glm::vec2(-cameraMovement * someX, 0.0f);
      }
      if (cursor.y > borderValue)
      {
         float someY = (cursor.y - borderValue) * multiplier;
         cameraMoveBy += glm::vec2(0.0f, -cameraMovement * someY);
      }
      else if (cursor.y < -borderValue)
      {
         float someY = (cursor.y + borderValue) * multiplier;
         cameraMoveBy += glm::vec2(0.0f, -cameraMovement * someY);
      }
      if (glm::length(glm::vec2(cameraMoveBy)))
      {
         m_currentLevel.Move(cameraMoveBy);
         m_player->Move(cameraMoveBy);
      }
   }
}

void
Game::RenderFirstPass()
{
   m_frameBuffer.BeginDrawingToTexture();

   if (!m_reverse)
   {
      // player's position on the map
      m_playerPosition = m_currentLevel.GetLocalVec(m_player->GetCenteredGlobalPosition());

      glm::ivec2 correction = CorrectPosition();

      m_player->Move(correction);
      m_playerPosition += correction;
      m_player->SetCenteredLocalPosition(m_playerPosition);

      m_currentLevel.Draw(*m_window, m_frameCount);
      m_player->Render(*m_window, m_frameCount);
   }
   else
   {
      m_currentLevel.DrawReverse(*m_window, m_frameCount);
      m_player->RenderReverse(*m_window, m_frameCount);
   }

   m_frameBuffer.EndDrawingToTexture();
}

void
Game::RenderSecondPass()
{
   m_frameBuffer.DrawFrameBuffer();

   glm::ivec2 debug2 = m_player->GetCenteredLocalPosition();
   for (auto& obj : m_debugObjs)
   {
      obj->Draw(m_window->GetProjection());
   }
   m_debugObjs.clear();

   RenderText(std::to_string(m_deltaTime * 1000) + " ms",
              glm::vec2(static_cast< float >(-WIDTH / 2), static_cast< float >(-HEIGHT / 2) + 20), 0.4f, glm::vec3(1.0f, 0.0f, 1.0f));
}

void
Game::LoadLevel(const std::string& levelName)
{
   std::filesystem::path folderPath = ASSETS_DIR / levelName;
   std::ifstream levelFile((folderPath / levelName).u8string() + ".txt");
   if (!levelFile)
   {
      logger.Log(Logger::TYPE::FATAL, "Can't open " + (folderPath / levelName).u8string());
   }

   int32_t levelWidth(0), levelHeight(0);
   std::string background;
   std::string collisionMap;

   while (!levelFile.eof())
   {
      std::string token = "";
      levelFile >> token;
      if (token == "Size:")
      {
         levelFile >> levelWidth;
         levelFile >> levelHeight;
      }
      else if (token == "Background:")
      {
         levelFile >> background;
         m_currentLevel.LoadPremade((folderPath / background).u8string(), glm::ivec2(levelWidth, levelHeight));
      }
      else if (token == "Objects:")
      {
         // TODO KAPPA
      }
      else if (token == "Particles:")
      {
         // TODO KAPPA
      }
      else if (token == "First_pass_shaders:")
      {
         std::string shadersName;
         levelFile >> shadersName;
         m_currentLevel.LoadShaders(shadersName);
      }
      else if (token == "Second_pass_shaders:")
      {
         std::string shadersName;
         levelFile >> shadersName;
         m_frameBuffer.LoadShaders(shadersName);
      }
      else if (token == "Collision:")
      {
         levelFile >> collisionMap;
         int32_t width, height, n;
         byte_vec4* tmpCollision =
            reinterpret_cast< byte_vec4* >(stbi_load((folderPath / collisionMap).u8string().c_str(), &width, &height, &n, 0));
         m_collision = std::unique_ptr< byte_vec4 >(tmpCollision);
      }
      else if (token == "Player:")
      {
         glm::vec2 position;
         levelFile >> position.x;
         levelFile >> position.y;

         glm::ivec2 size;
         levelFile >> size.x;
         levelFile >> size.y;

         std::string textureName, shaderName;
         levelFile >> textureName;
         levelFile >> shaderName;

         m_player = std::make_unique< Player >(*this, position, size, (folderPath / textureName).u8string());
         m_player->LoadShaders(shaderName);

         m_playerPosition = glm::ivec2(position.x, -position.y);
      }
      else if (token == "Enemies:")
      {
         int32_t numEnemies;
         levelFile >> numEnemies;
         for (int32_t i = 0; i < numEnemies; ++i)
         {
            glm::vec2 position;
            levelFile >> position.x;
            levelFile >> position.y;

            glm::ivec2 size;
            levelFile >> size.x;
            levelFile >> size.y;

            glm::vec2 centeredPosition{position.x - (size.x / 2.0f), position.y + (size.y / 2.0f)};
            auto globalPosition = m_currentLevel.GetGlobalVec(centeredPosition);

            std::string textureName, shaderName;
            levelFile >> textureName;
            m_currentLevel.AddGameObject(*this, globalPosition, size, (folderPath / textureName).u8string());
            levelFile >> shaderName;
         }
      }
   }

   m_currentLevel.Move(-m_player->GetCenteredGlobalPosition());
   m_player->Move(-m_player->GetCenteredGlobalPosition());
}

void
Game::ProcessInput(float deltaTime)
{
   m_deltaTime = deltaTime;

   MouseEvents(deltaTime);
   KeyEvents(deltaTime);
}

void
Game::RenderText(std::string text, const glm::vec2& position, float scale, const glm::vec3& color)
{
   m_font.RenderText(*m_window, text, position, scale, color);
}

void
Game::Render()
{
   RenderFirstPass();
   RenderSecondPass();

   if(m_reverse)
   {
      if(m_frameCount == 0)
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
      if(m_frameCount == NUM_FRAMES_TO_SAVE - 1)
      {
         // do nothing
      }
      else
      {
         ++m_frameCount;
      }

   }

 //  m_frameCount = m_reverse ? (m_frameCount == 0) ? 0 : --m_frameCount : (m_frameCount == NUM_FRAMES_TO_SAVE - 1) ? NUM_FRAMES_TO_SAVE : ++m_frameCount;
}
