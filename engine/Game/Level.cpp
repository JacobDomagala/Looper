#include "Level.hpp"
#include "Enemy.hpp"
#include "FileManager.hpp"
#include "Game.hpp"
#include "Player.hpp"
#include "Shaders.hpp"
#include "Timer.hpp"
#include "Window.hpp"

#include <fstream>
#include <functional>
#include <nlohmann/json.hpp>

void
Level::Create(const glm::ivec2& size)
{
   m_levelSize = size;
   m_background.SetSprite(glm::vec2(0.0f, 0.0f), m_levelSize);
   m_shaders.LoadDefault();
}

void
Level::Load(const std::string& pathToLevel, bool isGame)
{
   const auto json = FileManager::LoadJsonFile(pathToLevel);

   for (auto& [key, value] : json.items())
   {
      if (key == "BACKGROUND")
      {
         const auto background = json[key]["texture"];
         const auto width = json[key]["width"];
         const auto height = json[key]["height"];

         LoadPremade((IMAGES_DIR / std::string(background)).u8string(), glm::ivec2(width, height));

         // Temporary solution
         // This should be moved to Level class soon
         const auto collision = json[key]["collision"];
         std::unique_ptr< byte_vec4 > collisionMap(
            reinterpret_cast< byte_vec4* >(FileManager::LoadPictureRawBytes((IMAGES_DIR / std::string(collision)).u8string())));
         m_game.SetCollisionMap(std::move(collisionMap));
      }
      else if (key == "SHADER")
      {
         const auto shaderName = json[key]["name"];
         LoadShaders(shaderName);
      }
      else if (key == "PLAYER")
      {
         const auto position = json[key]["position"];
         const auto width = json[key]["width"];
         const auto height = json[key]["height"];
         const auto texture = json[key]["texture"];
         const auto weapons = json[key]["weapons"];

         m_player = std::make_shared< Player >(m_game, glm::vec2(position[0], position[1]), glm::ivec2(width, height), texture, isGame);
      }
      else if (key == "ENEMIES")
      {
         for (auto& enemy : json[key])
         {
            const auto position = enemy["position"];
            const auto width = enemy["width"];
            const auto height = enemy["height"];
            const auto texture = enemy["texture"];
            const auto weapons = enemy["weapons"];
            const auto animatePos = enemy["animate positions"];

            m_objects.emplace_back(
               std::make_unique< Enemy >(m_game, glm::vec2(position[0], position[1]), glm::ivec2(width, height), texture));
         }
      }
      else
      {
         m_logger.Log(Logger::TYPE::WARNING, "Level::Load -> Unspecified type " + key + " during level loading");
      }
   }
}

void
Level::Save(const std::string& pathToLevel)
{
   nlohmann::json json;
   FileManager::SaveJsonFile(pathToLevel, json);
}

glm::vec2
Level::GetLocalVec(const glm::vec2& global) const
{
   // get the vector relative to map's position
   glm::vec2 returnVal{m_background.GetPosition() - global};

   // change 'y' to originate in top left
   returnVal.y -= m_levelSize.y;
   returnVal *= -1;

   return returnVal;
}

glm::vec2
Level::GetGlobalVec(const glm::vec2& local) const
{
   glm::vec2 returnVal = local;

   returnVal *= -1;
   returnVal.y += m_levelSize.y;
   returnVal = m_background.GetPosition() - returnVal;

   return returnVal;
}

bool
Level::CheckCollision(const glm::ivec2& localPos, const Player& player)
{
   for (auto& obj : m_objects)
   {
      float length = glm::length(glm::vec2(localPos - obj->GetCenteredLocalPosition()));
      glm::vec2 objPos = obj->GetLocalPosition();
      glm::vec2 objSize = obj->GetSize();

      if (length < objSize.x / 2.5f)
      {
         obj->Hit(player.GetWeaponDmg());

         return false;
      }

      obj->SetColor({1.0f, 1.0f, 1.0f});
   }
   return true;
}

void
Level::LoadPremade(const std::string& fileName, const glm::ivec2& size)
{
   m_locked = false;
   m_cameraPosition = glm::vec2(0.0f, 0.0f);
   m_cameraTilePos = glm::ivec2(0, 0);
   m_levelSize = size;

   m_background.SetSpriteTextured(glm::vec2(0, 0), size, fileName);
   m_shaders.LoadDefault();
}

void
Level::LoadShaders(const std::string& shaderName)
{
   m_shaders.LoadShaders(shaderName + "_vs.glsl", shaderName + "_fs.glsl");
}

void
Level::AddGameObject(Game& game, const glm::vec2& pos, const glm::ivec2& size, const std::string& sprite)
{
   std::unique_ptr< GameObject > object = std::make_unique< Enemy >(game, pos, size, sprite);
   m_objects.push_back(std::move(object));
}

void
Level::Move(const glm::vec2& moveBy)
{
   for (auto& obj : m_objects)
   {
      obj->Move(moveBy);
   }

   m_background.Translate(moveBy);
   MoveCamera(moveBy);
}


void
Level::Update(bool isReverse)
{
   m_background.Update(isReverse);

   for (auto& obj : m_objects)
   {
      if (obj->Visible())
      {
         obj->DealWithPlayer();
         obj->Update(isReverse);
      }
   }
}

void
Level::Render(const glm::mat4& projectionMat)
{
   // draw background
   m_background.Render(projectionMat, m_shaders);

   for (auto& obj : m_objects)
   {
      if (obj->Visible())
      {
         obj->Render(projectionMat, m_shaders);
      }
   }

   if (m_player)
   {
      m_player->Render(projectionMat);
   }
}

void
Level::MoveObjs(const glm::vec2& moveBy, bool isCameraMovement)
{
   for (auto& obj : m_objects)
   {
      obj->Move(moveBy, isCameraMovement);
   }
}

void
Level::SetPlayersPosition(const glm::vec2& position)
{
   // m_playerPos = position;
   // m_playerPos /= m_tileSize;
}

void
Level::MoveCamera(const glm::vec2& moveBy)
{
   m_cameraPosition -= moveBy;
   // cameraTilePos = GetTilePosition(cameraPosition);
}

glm::ivec2
Level::CheckMoveCamera(const glm::vec2& moveBy) const
{
   glm::vec2 tmp = m_cameraPosition;
   tmp -= moveBy;

   return GetTilePosition(tmp);
}

glm::ivec2
Level::GetTilePosition(const glm::vec2& position) const
{
   float tmpX = position.x;
   float tmpY = position.y;

   int32_t tileX = static_cast< int32_t >(ceilf(tmpX / static_cast< float >(m_tileSize.x)));
   int32_t tileY = static_cast< int32_t >(ceilf(tmpY / static_cast< float >(m_tileSize.y)));

   return glm::ivec2(tileX, tileY);
}
