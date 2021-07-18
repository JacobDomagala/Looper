#include "Level.hpp"
#include "Enemy.hpp"
#include "FileManager.hpp"
#include "Game.hpp"
#include "Player.hpp"
#include "Shader.hpp"
#include "Timer.hpp"
#include "Window.hpp"

#include <algorithm>
#include <fstream>
#include <functional>
#include <nlohmann/json.hpp>
#include <set>

namespace dgame {

void
Level::Create(Application* context, const glm::ivec2& size)
{
   m_levelSize = size;
   const auto halfSize = size / 2;
   m_background.SetSprite(glm::vec2(halfSize), m_levelSize);
   m_contextPointer = context;
   m_pathFinder.Initialize(m_levelSize, m_tileWidth);
}

void
Level::Load(Application* context, const std::string& pathToLevel)
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
         auto gameHandle = static_cast< Game* >(context);
         if (gameHandle)
         {
            const auto collision = json[key]["collision"];
            m_collision.LoadTextureFromFile(collision);
            gameHandle->SetCollisionMap(m_collision.GetVec4Data());
         }

         m_contextPointer = context;
      }
      else if (key == "PATHFINDER")
      {
         m_pathFinder.InitializeEmpty(m_levelSize, m_tileWidth);

         for (auto& nodeJson : json[key]["nodes"])
         {
            m_pathFinder.AddNode(Node(glm::ivec2(nodeJson["coords"][0], nodeJson["coords"][1]),
                                      glm::ivec2(nodeJson["position"][0], nodeJson["position"][1]),
                                      nodeJson["id"],
                                      std::vector< Node::NodeID >(nodeJson["connected to"].begin(),
                                                                  nodeJson["connected to"].end()),
                                      nodeJson["occupied"]));
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
         const auto name = json[key]["name"];

         m_player = std::make_shared< Player >(*context, glm::vec2(position[0], position[1]),
                                               glm::ivec2(size[0], size[1]), texture, name);
         m_player->GetSprite().Scale(glm::vec2(json[key]["scale"][0], json[key]["scale"][1]));
         m_player->GetSprite().Rotate(json[key]["rotation"]);
         m_objects.emplace_back(m_player);
      }
      else if (key == "ENEMIES")
      {
         for (auto& enemy : json[key])
         {
            const auto position = enemy["position"];
            const auto size = enemy["size"];
            const auto texture = enemy["texture"];
            const auto weapons = enemy["weapons"];
            const auto name = enemy["name"];

            auto object = std::make_shared< Enemy >(*context, glm::vec2(position[0], position[1]),
                                                    glm::ivec2(size[0], size[1]), texture,
                                                    std::vector< AnimationPoint >{});
            object->SetName(name);
            object->GetSprite().Scale(glm::vec2(enemy["scale"][0], enemy["scale"][1]));
            object->GetSprite().Rotate(enemy["rotation"]);

            std::vector< AnimationPoint > keypointsPositions = {};
            glm::vec2 beginPoint = glm::vec2(position[0], position[1]);

            for (auto& point : enemy["animate positions"])
            {
               AnimationPoint animationPoint(object->GetID());
               animationPoint.m_end = glm::vec2(point["end position"][0], point["end position"][1]);
               animationPoint.m_timeDuration = Timer::seconds(point["time duration"]);

               keypointsPositions.emplace_back(animationPoint);

               beginPoint = animationPoint.m_end;
            }

            object->SetAnimationKeypoints(keypointsPositions);
            m_objects.emplace_back(object);
         }
      }
      else if (key == "OBJECTS")
      {
         for (auto& object : json[key])
         {
            const auto position = object["position"];
            const auto size = object["size"];
            const auto texture = object["texture"];
            const auto name = object["name"];

            auto gameObject = std::make_shared< GameObject >(
               *context, glm::vec2(position[0], position[1]), glm::ivec2(size[0], size[1]), texture,
               Object::TYPE::OBJECT);
            gameObject->SetName(name);
            gameObject->GetSprite().Scale(glm::vec2(object["scale"][0], object["scale"][1]));
            gameObject->GetSprite().Rotate(object["rotation"]);
            gameObject->SetHasCollision(object["has collision"]);

            m_objects.emplace_back(gameObject);
         }
      }
      else
      {
         m_logger.Log(Logger::TYPE::FATAL,
                      "Level::Load -> Unspecified type " + key + " during level loading");
      }
   }
}

void
Level::Save(const std::string& pathToLevel)
{
   nlohmann::json json;

   const auto nodes = m_pathFinder.GetAllNodes();
   for (const auto& node : nodes)
   {
      nlohmann::json nodeJson;
      nodeJson["id"] = node.m_ID;
      nodeJson["coords"] = {node.m_xPos, node.m_yPos};
      nodeJson["position"] = {node.m_position.x, node.m_position.y};
      nodeJson["connected to"] = node.m_connectedNodes;
      nodeJson["occupied"] = node.m_occupied;

      json["PATHFINDER"]["nodes"].emplace_back(nodeJson);
   }

   // Serialize shader
   // json["SHADER"]["name"] = m_shaders.GetName();

   // Serialize background
   json["BACKGROUND"]["texture"] = m_background.GetTextureName();
   json["BACKGROUND"]["size"] = {m_background.GetSize().x, m_background.GetSize().y};
   json["BACKGROUND"]["collision"] = m_collision.GetName();

   // Serialize game objects
   for (const auto& object : m_objects)
   {
      switch (object->GetType())
      {
         case Object::TYPE::PLAYER: {
            json["PLAYER"]["name"] = m_player->GetName();
            json["PLAYER"]["position"] = {m_player->GetLocalPosition().x,
                                          m_player->GetLocalPosition().y};
            json["PLAYER"]["scale"] = {m_player->GetSprite().GetScale().x,
                                       m_player->GetSprite().GetScale().y};
            json["PLAYER"]["rotation"] = m_player->GetSprite().GetRotation();
            json["PLAYER"]["size"] = {m_player->GetSprite().GetOriginalSize().x,
                                      m_player->GetSprite().GetOriginalSize().y};
            json["PLAYER"]["texture"] = m_player->GetSprite().GetTextureName();
            json["PLAYER"]["weapons"] = m_player->GetWeapons();
         }
         break;

         case Object::TYPE::ENEMY: {
            nlohmann::json enemyJson;

            enemyJson["name"] = object->GetName();
            enemyJson["position"] = {object->GetLocalPosition().x, object->GetLocalPosition().y};
            enemyJson["size"] = {object->GetSprite().GetOriginalSize().x,
                                 object->GetSprite().GetOriginalSize().y};
            enemyJson["scale"] = {object->GetSprite().GetScale().x,
                                  object->GetSprite().GetScale().y};
            enemyJson["rotation"] = object->GetSprite().GetRotation();
            enemyJson["texture"] = object->GetSprite().GetTextureName();

            auto enemyPtr = dynamic_cast< Enemy* >(object.get());

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

         case Object::TYPE::OBJECT: {
            nlohmann::json objectJson;

            objectJson["name"] = object->GetName();
            objectJson["has collision"] = object->GetHasCollision();

            const auto occupiedNodes = object->GetOccupiedNodes();
            for (auto& node : occupiedNodes)
            {
               nlohmann::json occupiedNode;
               occupiedNode["tile position"] = {node.first, node.second};

               objectJson["occupied nodes"].emplace_back(occupiedNode);
            }

            objectJson["position"] = {object->GetLocalPosition().x, object->GetLocalPosition().y};
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
            m_logger.Log(Logger::TYPE::WARNING,
                         "Unhandled Object type {} present in game level file!",
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
Level::AddGameObject(GameObject::TYPE objectType)
{
   const auto defaultPosition = GetLocalVec(m_contextPointer->GetCamera().GetPosition());
   const auto defaultSize = glm::ivec2(128, 128);
   const auto defaultTexture = std::string("Default128.png");

   std::shared_ptr< GameObject > newObject;

   switch (objectType)
   {
      case GameObject::TYPE::ENEMY: {
         newObject = std::make_shared< Enemy >(*m_contextPointer, defaultPosition, defaultSize,
                                               defaultTexture, std::vector< AnimationPoint >{});
         m_objects.push_back(newObject);
      }
      break;

      case GameObject::TYPE::PLAYER: {
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

      case GameObject::TYPE::OBJECT: {
         newObject = std::make_shared< GameObject >(*m_contextPointer, defaultPosition, defaultSize,
                                                    defaultTexture, Object::TYPE::OBJECT);

         m_objects.push_back(newObject);
      }
      break;

      default: {
      }
   }

   return newObject;
}

glm::vec2
Level::GetLocalVec(const glm::vec2& global) const
{
   // get the vector relative to map's position
   // glm::vec2 returnVal{m_background.GetPosition() - global};

   //// change 'y' to originate in top left
   // returnVal.y -= m_levelSize.y;
   // returnVal *= -1;

   return global;
}

glm::vec2
Level::GetGlobalVec(const glm::vec2& local) const
{
   // glm::vec2 returnVal = local;

   // returnVal *= -1;
   // returnVal.y += m_levelSize.y;
   // returnVal = m_background.GetPosition() - returnVal;

   return local;
}

std::vector< std::pair< int32_t, int32_t > >
Level::GameObjectMoved(const std::array< glm::vec2, 4 >& box,
                       const std::vector< std::pair< int32_t, int32_t > >& currentTiles)
{
   auto new_tiles = GetTilesFromBoundingBox(box);

   if (m_pathFinder.IsInitialized())
   {
      for (auto tileID : currentTiles)
      {
         m_pathFinder.SetNodeFreed(tileID);
      }

      for (auto tileID : new_tiles)
      {
         m_pathFinder.SetNodeOccupied(tileID);
      }
   }

   return new_tiles;
}

std::vector< std::pair< int32_t, int32_t > >
Level::GetTilesFromBoundingBox(const std::array< glm::vec2, 4 >& box) const
{
   /**
    * Given 2 convex shapes:
    * 1. Calculate perpndicular vector to one of the side and normalize it
    * 2. Loop through every point on the first polygon and project it onto the axis
    *    (Keep track of the highest and lowest values found for this polygon)
    * 3. Do the same for the second polygon.
    */

   /*  let axis = {
     x : -(vertices[1].y - vertices[0].y),
     y : vertices[1].x - vertices[0].x
  }*/

   // const auto axis = glm::normalize(glm::vec2(-(box[0].y - box[3].y), box[0].x - box[3].x));

   // For both shapes

   // const auto topRightTile = GetTileFromPosition(box[0]);
   // const auto topLeftTile = GetTileFromPosition(box[1]);
   // const auto bottomLeftTile = GetTileFromPosition(box[2]);
   // const auto bottomRightTile = GetTileFromPosition(box[3]);

   float minX = std::numeric_limits< float >::max();
   float maxX = std::numeric_limits< float >::min();

   float minY = std::numeric_limits< float >::max();
   float maxY = std::numeric_limits< float >::min();

   for (const auto val : box)
   {
      minX = glm::min(minX, val.x);
      maxX = glm::max(maxX, val.x);

      minY = glm::min(minY, val.y);
      maxY = glm::max(maxY, val.y);
   }

   const auto minTileID = GetTileFromPosition(glm::vec2{minX, minY});
   const auto maxTileID = GetTileFromPosition(glm::vec2{maxX, maxY});

   std::vector< std::pair< int32_t, int32_t > > ret;

   for (int y = minTileID.second; y <= maxTileID.second; ++y)
   {
      for (int x = minTileID.first; x <= maxTileID.first; ++x)
      {
         ret.push_back({x, y});
      }
   }

   return ret;
}

std::pair< int32_t, int32_t >
Level::GetTileFromPosition(const glm::vec2& local) const
{
   if (!IsInLevelBoundaries(local))
   {
      return {-1, -1};
   }

   const auto w = glm::floor(local.x / static_cast< float >(m_tileWidth));
   const auto h = glm::floor(local.y / static_cast< float >(m_tileWidth));

   return {w, h};
}

bool
Level::IsInLevelBoundaries(const glm::vec2& position) const
{
   return position.x >= 0 && position.x <= static_cast< float >(m_levelSize.x) && position.y >= 0
          && position.y <= static_cast< float >(m_levelSize.y);
}

bool
Level::CheckCollision(const glm::ivec2& localPos, const Player& player)
{
   for (auto& obj : m_objects)
   {
      auto length = glm::length(glm::vec2(localPos - obj->GetCenteredLocalPosition()));
      auto objSize = obj->GetSize();

      if (length < static_cast< float >(objSize.x) / 2.5f)
      {
         obj->Hit(player.GetWeaponDmg());

         return false;
      }

      obj->SetColor({1.0f, 1.0f, 1.0f});
   }
   return true;
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
      if (m_pathFinder.GetNodeFromPosition(curPos).m_occupied)
      {
         noCollision = false;
         break;
      }
   }

   return noCollision;
}

void
Level::LoadPremade(const std::string& fileName, const glm::ivec2& size)
{
   m_locked = false;
   m_levelSize = size;

   m_background.SetSpriteTextured(glm::vec2(static_cast< float >(m_levelSize.x) / 2.0f,
                                            static_cast< float >(m_levelSize.y) / 2.0f),
                                  size, fileName);
   // m_shaders.LoadDefault();
}

void
Level::LoadShaders(const std::string&)
{
   // m_shaders.LoadShaders(shaderName);
}

void
Level::AddGameObject(Game& game, const glm::vec2& pos, const glm::ivec2& size,
                     const std::string& sprite)
{
   std::unique_ptr< GameObject > object = std::make_unique< Enemy >(game, pos, size, sprite);
   m_objects.push_back(std::move(object));
}

void
Level::DeleteObject(std::shared_ptr< Object > deletedObject)
{
   m_objects.erase(std::find(m_objects.begin(), m_objects.end(), deletedObject));
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
      m_logger.Log(Logger::TYPE::FATAL,
                   "Trying to delete an object that doesn't exist! Object type: {}",
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
      case Object::TYPE::ENEMY: {
         auto it = std::find_if(m_objects.begin(), m_objects.end(), [objectID](const auto& object) {
            return object->GetID() == objectID;
         });

         if (it != m_objects.end())
         {
            requestedObject = (*it).get();
         }
      }
      break;

      case Object::TYPE::PLAYER: {
         // Assume it's the player
         return *m_player;
      }
      break;

      case Object::TYPE::ANIMATION_POINT: {
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

      case Object::TYPE::PATHFINDER_NODE: {
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
         m_logger.Log(Logger::TYPE::FATAL, "Trying to get Object on unknown type!");
      }
   }

   // This should never happen
   assert(requestedObject);

   return *requestedObject;
}

void
Level::Move(const glm::vec2& moveBy)
{
   for (auto& obj : m_objects)
   {
      obj->Move(moveBy);
   }

   m_background.Translate(moveBy);
   // MoveCamera(moveBy);
}

void
Level::Scale(const glm::vec2& scaleVal)
{
   for (auto& obj : m_objects)
   {
      obj->Scale(scaleVal);
   }

   m_background.Scale(scaleVal);
}

void
Level::Rotate(float angle, bool cumulative)
{
   for (auto& obj : m_objects)
   {
      obj->Rotate(angle, cumulative);
   }

   cumulative ? m_background.RotateCumulative(angle) : m_background.Rotate(angle);
}

void
Level::Update(bool isReverse)
{
   m_background.Update(isReverse);

   for (auto& obj : m_objects)
   {
      if (obj->Visible())
      {
         if (obj->GetType() == Object::TYPE::ENEMY)
         {
            std::dynamic_pointer_cast< Enemy >(obj)->DealWithPlayer();
         }

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
Level::MoveObjs(const glm::vec2& moveBy, bool isCameraMovement)
{
   for (auto& obj : m_objects)
   {
      obj->Move(moveBy, isCameraMovement);
   }
}

std::vector< std::shared_ptr< GameObject > >
Level::GetObjects(bool)
{
   return m_objects;
}

void
Level::SetPlayersPosition(const glm::vec2&)
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

Sprite&
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

   m_background.Translate(-diff);
}

} // namespace dgame
