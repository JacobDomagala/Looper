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
Level::Load(Context& context, const std::string& pathToLevel)
{
   const auto json = FileManager::LoadJsonFile(pathToLevel);

   for (auto& [key, value] : json.items())
   {
      if (key == "BACKGROUND")
      {
         const auto background = json[key]["texture"];
         const auto size = json[key]["size"];

         LoadPremade(background, glm::ivec2(size[0], size[1]));

         // Temporary solution
         // This should be moved to Level class soon
         auto gameHandle = static_cast< Game* >(&context);
         if (gameHandle)
         {
            const auto collision = json[key]["collision"];
            m_collision.LoadTextureFromFile(collision);
            gameHandle->SetCollisionMap(reinterpret_cast< byte_vec4* >(m_collision.GetData()));
         }
      }
      else if (key == "SHADER")
      {
         const auto shaderName = json[key]["name"];
         LoadShaders(shaderName);
      }
      else if (key == "PLAYER")
      {
         const auto position = json[key]["position"];
         const auto size = json[key]["size"];
         const auto texture = json[key]["texture"];
         const auto weapons = json[key]["weapons"];

         m_player = std::make_shared< Player >(context, glm::vec2(position[0], position[1]), glm::ivec2(size[0], size[1]), texture);
      }
      else if (key == "ENEMIES")
      {
         if (json[key].is_array())
         {
            for (auto& enemy : json[key])
            {
               const auto position = enemy["position"];
               const auto size = enemy["size"];
               const auto texture = enemy["texture"];
               const auto weapons = enemy["weapons"];
               //   const auto animatePos = enemy["animate positions"];

               m_objects.emplace_back(
                  std::make_unique< Enemy >(context, glm::vec2(position[0], position[1]), glm::ivec2(size[0], size[1]), texture));
            }
         }
         else
         {
            const auto position = json[key]["position"];
            const auto size = json[key]["size"];
            const auto texture = json[key]["texture"];
            const auto weapons = json[key]["weapons"];
            //   const auto animatePos = json[key]["animate positions"];

            m_objects.emplace_back(
               std::make_unique< Enemy >(context, glm::vec2(position[0], position[1]), glm::ivec2(size[0], size[1]), texture));
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

   // Serialize shader
   json["SHADER"]["name"] = m_shaders.GetName();

   // Serialize background
   json["BACKGROUND"]["texture"] = m_background.GetTextureName();
   json["BACKGROUND"]["size"] = {m_background.GetSize().x, m_background.GetSize().y};
   json["BACKGROUND"]["collision"] = m_collision.GetName();

   // Serialize player
   json["PLAYER"]["position"] = {m_player->GetLocalPosition().x, m_player->GetLocalPosition().y};
   json["PLAYER"]["size"] = {m_player->GetSize().x, m_player->GetSize().y};
   json["PLAYER"]["texture"] = m_player->GetSprite().GetTextureName();
   json["PLAYER"]["weapons"] = m_player->GetWeapons();

   // Serialize game objects
   for (const auto& object : m_objects)
   {
      json["ENEMIES"]["position"] = {object->GetLocalPosition().x, object->GetLocalPosition().y};
      json["ENEMIES"]["size"] = {object->GetSize().x, object->GetSize().y};
      json["ENEMIES"]["texture"] = object->GetSprite().GetTextureName();
      json["ENEMIES"]["weapons"] = dynamic_cast< Enemy* >(object.get())->GetWeapon();
   }

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
   m_shaders.LoadShaders(shaderName);
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
      m_player->Render(projectionMat, m_shaders);
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
