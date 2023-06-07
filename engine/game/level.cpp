#include "level.hpp"
#include "enemy.hpp"
#include "game.hpp"
#include "player.hpp"
#include "renderer/renderer.hpp"
#include "renderer/window/window.hpp"
#include "utils/file_manager.hpp"
#include "utils/time/timer.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>
#include <functional>
#include <set>

namespace looper {

void
Level::Create(Application* context, const std::string& name, const glm::ivec2& size)
{
   m_name = name;
   m_levelSize = size;

   m_background.SetSpriteTextured(glm::vec3(static_cast< float >(m_levelSize.x) / 2.0f,
                                            static_cast< float >(m_levelSize.y) / 2.0f, 0.3f),
                                  size, "white.png");

   m_contextPointer = context;
   m_pathFinder.Initialize(this);
}

void
Level::Load(Application* context, const std::string& pathToLevel)
{
   const auto json = FileManager::LoadJsonFile(pathToLevel);

   for (const auto& [key, value] : json.items())
   {
      if (key == "BACKGROUND")
      {
         const auto background = json[key]["texture"];
         const auto size = json[key]["size"];

         LoadPremade(background, glm::ivec2(size[0], size[1]));

         m_contextPointer = context;
      }
      else if (key == "PATHFINDER")
      {
         m_pathFinder.InitializeEmpty(this);

         for (const auto& nodeJson : json[key]["nodes"])
         {
            m_pathFinder.AddNode(Node(glm::ivec2(nodeJson["coords"][0], nodeJson["coords"][1]),
                                      glm::ivec2(nodeJson["position"][0], nodeJson["position"][1]),
                                      nodeJson["id"],
                                      std::vector< NodeID >(nodeJson["connected to"].begin(),
                                                            nodeJson["connected to"].end()),
                                      nodeJson["occupied"],
                                      std::vector< Object::ID >(nodeJson["nodesOccupying"].begin(),
                                                                nodeJson["nodesOccupying"].end())));
         }

         m_pathFinder.SetInitialized();
      }
      else if (key == "SHADER")
      {
         const auto shaderName = json[key]["name"];
         LoadShaders(shaderName);
      }
      else if (key == "PLAYER")
      {
         const auto& player = json[key];
         const auto& position = player["position"];
         const auto& size = player["size"];
         const auto& texture = player["texture"];
         // const auto weapons = json[key]["weapons"];
         const auto& name = player["name"];

         m_player = std::make_shared< Player >(*context, glm::vec3(position[0], position[1], 0.0f),
                                               glm::ivec2(size[0], size[1]), texture, name);
         m_player->GetSprite().Scale(glm::vec2(player["scale"][0], player["scale"][1]));
         m_player->GetSprite().Rotate(player["rotation"]);
         m_player->SetID(player["id"]);
         m_objects.emplace_back(m_player);
      }
      else if (key == "ENEMIES")
      {
         for (const auto& enemy : json[key])
         {
            const auto& position = enemy["position"];
            const auto& size = enemy["size"];
            const auto& texture = enemy["texture"];
            // const auto weapons = enemy["weapons"];
            const auto& name = enemy["name"];

            auto object = std::make_shared< Enemy >(
               *context, glm::vec3(position[0], position[1], 0.0f), glm::ivec2(size[0], size[1]),
               texture, std::vector< AnimationPoint >{});
            object->SetName(name);
            object->SetID(enemy["id"]);
            object->GetSprite().Scale(glm::vec2(enemy["scale"][0], enemy["scale"][1]));
            object->GetSprite().Rotate(enemy["rotation"]);

            std::vector< AnimationPoint > keypointsPositions = {};
            glm::vec2 beginPoint = glm::vec2(position[0], position[1]);

            for (const auto& point : enemy["animate positions"])
            {
               AnimationPoint animationPoint(object->GetID());
               animationPoint.m_end = glm::vec2(point["end position"][0], point["end position"][1]);
               animationPoint.m_timeDuration = Timer::seconds(point["time duration"]);

               keypointsPositions.emplace_back(animationPoint);

               beginPoint = animationPoint.m_end;
            }

            object->SetAnimationKeypoints(std::move(keypointsPositions));
            m_objects.emplace_back(object);
         }
      }
      else if (key == "OBJECTS")
      {
         for (const auto& object : json[key])
         {
            const auto& position = object["position"];
            const auto& size = object["size"];
            const auto& texture = object["texture"];
            const auto& name = object["name"];

            auto gameObject = std::make_shared< GameObject >(
               *context, glm::vec3(position[0], position[1], 0.0f), glm::ivec2(size[0], size[1]),
               texture, ObjectType::OBJECT);
            gameObject->SetName(name);
            gameObject->SetID(object["id"]);
            gameObject->GetSprite().Scale(glm::vec2(object["scale"][0], object["scale"][1]));
            gameObject->GetSprite().Rotate(object["rotation"]);
            gameObject->SetHasCollision(object["has collision"]);

            m_objects.emplace_back(gameObject);
         }
      }
      else
      {
         Logger::Fatal("Level::Load -> Unspecified type {} during level loading", key);
      }
   }
}

void
Level::Save(const std::string& pathToLevel)
{
   nlohmann::json json;

   const auto& nodes = m_pathFinder.GetAllNodes();
   for (const auto& node : nodes)
   {
      nlohmann::json nodeJson;
      nodeJson["id"] = node.id_;
      nodeJson["coords"] = {node.xPos_, node.yPos_};
      nodeJson["position"] = {node.position_.x, node.position_.y};
      nodeJson["connected to"] = node.connectedNodes_;
      nodeJson["occupied"] = node.occupied_;
      nodeJson["nodesOccupying"] = node.objectsOccupyingThisNode_;

      json["PATHFINDER"]["nodes"].emplace_back(nodeJson);
   }

   // Serialize shader
   // json["SHADER"]["name"] = m_shaders.GetName();

   // Serialize background
   json["BACKGROUND"]["texture"] = m_background.GetTextureName();
   json["BACKGROUND"]["size"] = {m_background.GetSize().x, m_background.GetSize().y};

   // Serialize game objects
   for (const auto& object : m_objects)
   {
      const auto id = object->GetID();

      switch (object->GetType())
      {
         case ObjectType::PLAYER: {
            json["PLAYER"]["name"] = m_player->GetName();
            json["PLAYER"]["id"] = id;
            json["PLAYER"]["position"] = {m_player->GetPosition().x, m_player->GetPosition().y};
            json["PLAYER"]["scale"] = {m_player->GetSprite().GetScale().x,
                                       m_player->GetSprite().GetScale().y};
            json["PLAYER"]["rotation"] = m_player->GetSprite().GetRotation();
            json["PLAYER"]["size"] = {m_player->GetSprite().GetOriginalSize().x,
                                      m_player->GetSprite().GetOriginalSize().y};
            json["PLAYER"]["texture"] = m_player->GetSprite().GetTextureName();
            json["PLAYER"]["weapons"] = m_player->GetWeapons();
         }
         break;

         case ObjectType::ENEMY: {
            nlohmann::json enemyJson;

            enemyJson["name"] = object->GetName();
            enemyJson["id"] = id;
            enemyJson["position"] = {object->GetPosition().x, object->GetPosition().y};
            enemyJson["size"] = {object->GetSprite().GetOriginalSize().x,
                                 object->GetSprite().GetOriginalSize().y};
            enemyJson["scale"] = {object->GetSprite().GetScale().x,
                                  object->GetSprite().GetScale().y};
            enemyJson["rotation"] = object->GetSprite().GetRotation();
            enemyJson["texture"] = object->GetSprite().GetTextureName();

            auto* enemyPtr = dynamic_cast< Enemy* >(object.get());

            enemyJson["weapons"] = enemyPtr->GetWeapon();
            enemyJson["animation type"] =
               enemyPtr->GetAnimationType() == Animatable::ANIMATION_TYPE::LOOP ? "Loop"
                                                                                : "Reversable";

            const auto keypoints = enemyPtr->GetAnimationKeypoints();
            for (const auto& point : keypoints)
            {
               nlohmann::json animationPoint;
               animationPoint["end position"] = {point.m_end.x, point.m_end.y};
               animationPoint["time duration"] = point.m_timeDuration.count();

               enemyJson["animate positions"].emplace_back(animationPoint);
            }

            json["ENEMIES"].emplace_back(enemyJson);
         }
         break;

         case ObjectType::OBJECT: {
            nlohmann::json objectJson;

            objectJson["name"] = object->GetName();
            objectJson["id"] = id;
            objectJson["has collision"] = object->GetHasCollision();

            const auto occupiedNodes = object->GetOccupiedNodes();
            for (const auto& node : occupiedNodes)
            {
               nlohmann::json occupiedNode;
               occupiedNode["tile position"] = {node.first, node.second};

               objectJson["occupied nodes"].emplace_back(occupiedNode);
            }

            objectJson["position"] = {object->GetPosition().x, object->GetPosition().y};
            objectJson["size"] = {object->GetSprite().GetOriginalSize().x,
                                  object->GetSprite().GetOriginalSize().y};
            objectJson["scale"] = {object->GetSprite().GetScale().x,
                                   object->GetSprite().GetScale().y};
            objectJson["rotation"] = object->GetSprite().GetRotation();
            objectJson["texture"] = object->GetSprite().GetTextureName();

            json["OBJECTS"].emplace_back(objectJson);
         }
         break;

         default: {
            Logger::Warn("Level: Unhandled Object type {} present in game level file!",
                         object->GetTypeString());
         }
      }
   }

   FileManager::SaveJsonFile(pathToLevel, json);
}

void
Level::Quit()
{
   m_objects.clear();
   m_player.reset();
}

std::shared_ptr< GameObject >
Level::AddGameObject(ObjectType objectType)
{
   const auto defaultPosition = m_contextPointer->GetCamera().GetPosition();
   const auto defaultSize = glm::ivec2(128, 128);
   const auto defaultTexture = std::string("Default128.png");

   std::shared_ptr< GameObject > newObject;

   switch (objectType)
   {
      case ObjectType::ENEMY: {
         newObject = std::make_shared< Enemy >(*m_contextPointer, defaultPosition, defaultSize,
                                               defaultTexture, std::vector< AnimationPoint >{});
         m_objects.push_back(newObject);
      }
      break;

      case ObjectType::PLAYER: {
         if (m_player != nullptr)
         {
            newObject = m_player;
         }
         else
         {
            newObject = std::make_shared< Player >(*m_contextPointer, defaultPosition, defaultSize,
                                                   defaultTexture);
            m_player = std::dynamic_pointer_cast< Player >(newObject);
            m_objects.push_back(newObject);
         }
      }
      break;

      case ObjectType::OBJECT: {
         newObject = std::make_shared< GameObject >(*m_contextPointer, defaultPosition, defaultSize,
                                                    defaultTexture, ObjectType::OBJECT);

         m_objects.push_back(newObject);
      }
      break;

      default: {
      }
   }

   renderer::VulkanRenderer::UpdateBuffers();
   renderer::VulkanRenderer::CreateLinePipeline();

   return newObject;
}

std::vector< Tile >
Level::GameObjectMoved(const std::array< glm::vec2, 4 >& box,
                       const std::vector< Tile >& currentTiles, Object::ID objectID)
{
   auto newTiles = GetTilesFromBoundingBox(box);

   if (m_pathFinder.IsInitialized())
   {
      for (auto tileID : currentTiles)
      {
         m_pathFinder.SetNodeFreed(tileID, objectID);
      }

      for (auto tileID : newTiles)
      {
         m_pathFinder.SetNodeOccupied(tileID, objectID);
      }
   }

   return newTiles;
}

std::vector< Tile >
Level::GetTilesFromBoundingBox(const std::array< glm::vec2, 4 >& box) const
{
   std::vector< Tile > ret;

   auto insertToVec = [&ret](std::vector< Tile > tiles) {
      ret.insert(ret.end(), tiles.begin(), tiles.end());
   };

   insertToVec(GetTilesAlongTheLine(box[0], box[3]));
   insertToVec(GetTilesAlongTheLine(box[3], box[2]));
   insertToVec(GetTilesAlongTheLine(box[2], box[1]));
   insertToVec(GetTilesAlongTheLine(box[1], box[0]));

   return ret;
}

Tile
Level::GetTileFromPosition(const glm::vec2& local) const
{
   if (!IsInLevelBoundaries(local))
   {
      return INVALID_TILE;
   }

   const auto w = static_cast< int32_t >(glm::floor(local.x / static_cast< float >(m_tileWidth)));
   const auto h = static_cast< int32_t >(glm::floor(local.y / static_cast< float >(m_tileWidth)));

   return {w, h};
}

bool
Level::IsInLevelBoundaries(const glm::vec2& position) const
{
   return position.x >= 0 && position.x <= static_cast< float >(m_levelSize.x) && position.y >= 0
          && position.y <= static_cast< float >(m_levelSize.y);
}

glm::vec2
Level::GetCollidedPosition(const glm::vec2& fromPos, const glm::vec2& toPos)
{
   constexpr auto numSteps = 100;
   constexpr auto singleStep = 1 / static_cast< float >(numSteps);

   const auto pathVec = toPos - fromPos;
   const auto stepSize = pathVec * singleStep;

   glm::vec2 returnPosition = fromPos;

   for (int i = 0; i < numSteps; ++i)
   {
      auto curPos = fromPos + (stepSize * static_cast< float >(i));

      if (!IsInLevelBoundaries(curPos) or m_pathFinder.GetNodeFromPosition(curPos).occupied_)
      {
         break;
      }

      returnPosition = curPos;
   }

   return glm::clamp(returnPosition, glm::vec2{0.0f, 0.0f},
                     static_cast< glm::vec2 >(m_levelSize - 1));
}

bool
Level::CheckCollisionAlongTheLine(const glm::vec2& fromPos, const glm::vec2& toPos)
{
   bool noCollision = true;

   constexpr auto numSteps = 100;
   constexpr auto singleStep = 1 / static_cast< float >(numSteps);

   const auto pathVec = toPos - fromPos;
   const auto stepSize = pathVec * singleStep;

   for (int i = 0; i < numSteps; ++i)
   {
      const auto curPos = fromPos + (stepSize * static_cast< float >(i));
      if (!IsInLevelBoundaries(curPos) or m_pathFinder.GetNodeFromPosition(curPos).occupied_)
      {
         noCollision = false;
         break;
      }
   }

   return noCollision;
}

std::vector< Tile >
Level::GetTilesAlongTheLine(const glm::vec2& fromPos, const glm::vec2& toPos) const
{
   std::set< Tile > tiles;

   constexpr auto numSteps = 100;
   constexpr auto singleStep = 1 / static_cast< float >(numSteps);

   const auto pathVec = toPos - fromPos;
   const auto stepSize = pathVec * singleStep;

   for (int i = 0; i < numSteps; ++i)
   {
      tiles.insert(GetTileFromPosition(fromPos + (stepSize * static_cast< float >(i))));
   }

   return {tiles.begin(), tiles.end()};
}

void
Level::LoadPremade(const std::string& fileName, const glm::ivec2& size)
{
   m_locked = false;
   m_levelSize = size;

   m_background.SetSpriteTextured(glm::vec3(static_cast< float >(m_levelSize.x) / 2.0f,
                                            static_cast< float >(m_levelSize.y) / 2.0f, 0.5f),
                                  size, fileName);
}

void
Level::LoadShaders(const std::string& /*shaderName*/)
{
   // m_shaders.LoadShaders(shaderName);
}

void
Level::DeleteObject(Object::ID deletedObject)
{
   auto objectIter =
      std::find_if(m_objects.begin(), m_objects.end(), [deletedObject](const auto& object) {
         return object->GetID() == deletedObject;
      });

   if (objectIter == m_objects.end())
   {
      Logger::Fatal("Level: Trying to delete an object that doesn't exist! Object type: {}",
                    Object::GetTypeString(deletedObject));
   }
   else
   {
      m_objects.erase(objectIter);
   }
}

Object&
Level::GetObjectRef(Object::ID objectID)
{
   Object* requestedObject = nullptr;

   switch (Object::GetTypeFromID(objectID))
   {
      case ObjectType::ENEMY: {
         auto it = std::find_if(m_objects.begin(), m_objects.end(), [objectID](const auto& object) {
            return object->GetID() == objectID;
         });

         if (it != m_objects.end())
         {
            requestedObject = (*it).get();
         }
      }
      break;

      case ObjectType::PLAYER: {
         // Assume it's the player
         return *m_player;
      }
      break;

      case ObjectType::ANIMATION_POINT: {
         for (auto& object : m_objects)
         {
            auto animatePtr = std::dynamic_pointer_cast< Animatable >(object);
            if (animatePtr)
            {
               auto& points = animatePtr->GetAnimationKeypoints();
               auto it = std::find_if(points.begin(), points.end(), [objectID](const auto& point) {
                  return point.GetID() == objectID;
               });

               if (it != points.end())
               {
                  requestedObject = &(*it);
                  break;
               }
            }
         }
      }
      break;

      case ObjectType::PATHFINDER_NODE: {
         auto& nodes = m_pathFinder.GetAllNodes();
         auto it = std::find_if(nodes.begin(), nodes.end(),
                                [objectID](const auto& node) { return node.GetID() == objectID; });

         if (it != nodes.end())
         {
            requestedObject = &(*it);
         }
      }
      break;

      default: {
         Logger::Fatal("Level: Trying to get Object on unknown type!");
      }
   }

   // This should never happen
   // NOLINTNEXTLINE
   assert(requestedObject);

   return *requestedObject;
}

void
Level::Update(bool isReverse)
{
   m_background.Update(isReverse);

   // TODO: Parallelize for larger groups of objects
   // Player and Enemies should be handled by a single thread,
   // since they're dependent of eachother
   for (auto& obj : m_objects)
   {
      // We probably should update only objects that are
      // in some range of camera
      if (obj->Visible())
      {
         obj->Update(isReverse);
      }
   }
}

void
Level::Render()
{
   m_background.Render();
   RenderGameObjects();
}

void
Level::RenderGameObjects()
{
   for (auto& obj : m_objects)
   {
      if (obj->Visible())
      {
         obj->Render();
      }
   }
}

void
Level::MoveObjs(const glm::vec2& moveBy)
{
   for (auto& obj : m_objects)
   {
      obj->Move(moveBy);
   }
}

const std::vector< std::shared_ptr< GameObject > >&
Level::GetObjects() const
{
   return m_objects;
}

void
Level::SetPlayersPosition(const glm::vec2& /*position*/)
{
   // m_playerPos = position;
   // m_playerPos /= m_tileSize;
}

std::shared_ptr< GameObject >
Level::GetGameObjectOnLocation(const glm::vec2& screenPosition)
{
   // std::shared_ptr< GameObject > foundObject = nullptr;

   // if (m_player)
   //{
   //   foundObject = m_player->CheckIfCollidedScreenPosion(screenPosition) ? m_player : nullptr;

   //   if (foundObject)
   //   {
   //      return foundObject;
   //   }
   //}

   auto objectOnLocation =
      std::find_if(m_objects.begin(), m_objects.end(), [screenPosition](const auto& object) {
         return object->CheckIfCollidedScreenPosion(screenPosition);
      });

   return objectOnLocation != m_objects.end() ? *objectOnLocation : nullptr;
}

renderer::Sprite&
Level::GetSprite()
{
   return m_background;
}

void
Level::SetSize(const glm::ivec2& newSize)
{
   const auto oldSize = static_cast< glm::vec2 >(m_levelSize);
   m_levelSize = newSize;
   m_background.SetSize(newSize);
   // Make sure top left is always 0,0
   const auto diff = oldSize / 2.0f - static_cast< glm::vec2 >(newSize) / 2.0f;

   m_background.Translate(glm::vec3(-diff, 0.0f));
}

} // namespace looper
