#include "level.hpp"
#include "enemy.hpp"
#include "game.hpp"
#include "player.hpp"
#include "renderer/renderer.hpp"
#include "renderer/window/window.hpp"
#include "utils/file_manager.hpp"
#include "utils/time/scoped_timer.hpp"
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

   // BACKGROUND
   {
      const auto& background = json["BACKGROUND"];
      const auto& backgroundTex = background["texture"];
      const auto& size = background["size"];

      LoadPremade(backgroundTex, glm::ivec2(size[0], size[1]));

      m_contextPointer = context;
   }

   // PATHFINDER
   {
      SCOPED_TIMER(fmt::format("Loading Pathfinder"));
      m_pathFinder.Initialize(this);
   }

   // PLAYER
   {
      SCOPED_TIMER("Loading Player");

      const auto& player = json["PLAYER"];
      const auto& position = player["position"];
      const auto& size = player["size"];
      const auto& texture = player["texture"];
      // const auto weapons = json[key]["weapons"];
      const auto& name = player["name"];

      player_.Setup(context, glm::vec3(position[0], position[1], 0.0f),
                    glm::ivec2(size[0], size[1]), texture, name);

      player_.Scale(glm::vec2(player["scale"][0], player["scale"][1]));
      player_.Rotate(player["rotation"]);
      player_.GetSprite().ChangeRenderLayer(player["render_layer"]);
   }

   // ENEMIES
   {
      const auto& enemies = json["ENEMIES"];
      SCOPED_TIMER(fmt::format("Loading Enemies ({})", enemies.size()));
      // This is a magic number that we should figure out later
      enemies_.reserve(100);
      enemies_.resize(enemies.size());

      for (size_t i = 0; i < enemies.size(); ++i)
      {
         const auto& enemy = enemies.at(i);
         const auto& position = enemy["position"];
         const auto& size = enemy["size"];
         const auto& texture = enemy["texture"];
         const auto& name = enemy["name"];

         auto& object = enemies_.at(i);
         object.Setup(context, glm::vec3(position[0], position[1], 0.0f),
                      glm::ivec2(size[0], size[1]), texture, std::vector< AnimationPoint >{});
         object.SetName(name);
         object.Scale(glm::vec2(enemy["scale"][0], enemy["scale"][1]));
         object.Rotate(enemy["rotation"]);
         object.GetSprite().ChangeRenderLayer(enemy["render_layer"]);

         std::vector< AnimationPoint > keypointsPositions = {};
         glm::vec2 beginPoint = glm::vec2(position[0], position[1]);

         for (const auto& point : enemy["animate positions"])
         {
            AnimationPoint animationPoint(object.GetID());
            animationPoint.m_end = glm::vec2(point["end position"][0], point["end position"][1]);
            animationPoint.m_timeDuration = time::seconds(point["time duration"]);

            keypointsPositions.emplace_back(animationPoint);

            beginPoint = animationPoint.m_end;
         }

         object.SetAnimationKeypoints(std::move(keypointsPositions));
      }
   }

   // OBJECTS
   {
      const auto& objects = json["OBJECTS"];
      SCOPED_TIMER(fmt::format("Loading Objects ({})", objects.size()));
      // This is a magic number that we should figure out later
      objects_.reserve(10000);
      objects_.resize(objects.size());
      for (size_t i = 0; i < objects.size(); ++i)
      {
         const auto& object = objects.at(i);
         const auto& position = object["position"];
         const auto& size = object["size"];
         const auto& texture = object["texture"];
         const auto& name = object["name"];

         auto& gameObject = objects_.at(i);
         gameObject.Setup(context, glm::vec3(position[0], position[1], 0.0f),
                          glm::ivec2(size[0], size[1]), texture, ObjectType::OBJECT);
         objectToIdx_[gameObject.GetID()] = i;
         gameObject.SetName(name);
         gameObject.Scale(glm::vec2(object["scale"][0], object["scale"][1]));
         gameObject.Rotate(object["rotation"]);
         gameObject.GetSprite().ChangeRenderLayer(object["render_layer"]);
         gameObject.SetHasCollision(object["has collision"]);
      }
   }
}

void
Level::Save(const std::string& pathToLevel)
{
   nlohmann::json json;

   // Serialize background
   json["BACKGROUND"]["texture"] = m_background.GetTextureName();
   json["BACKGROUND"]["size"] = {m_background.GetSize().x, m_background.GetSize().y};

   // PLAYER
   json["PLAYER"]["name"] = player_.GetName();
   json["PLAYER"]["position"] = {player_.GetPosition().x, player_.GetPosition().y};
   json["PLAYER"]["scale"] = {player_.GetSprite().GetScale().x, player_.GetSprite().GetScale().y};
   json["PLAYER"]["rotation"] = player_.GetSprite().GetRotation();
   json["PLAYER"]["size"] = {player_.GetSprite().GetOriginalSize().x,
                             player_.GetSprite().GetOriginalSize().y};
   json["PLAYER"]["texture"] = player_.GetSprite().GetTextureName();
   json["PLAYER"]["weapons"] = player_.GetWeapons();
   json["PLAYER"]["render_layer"] = player_.GetSprite().GetRenderInfo().layer;

   // ENEMIES
   for (const auto& enemy : enemies_)
   {
      nlohmann::json enemyJson;

      enemyJson["name"] = enemy.GetName();
      enemyJson["position"] = {enemy.GetPosition().x, enemy.GetPosition().y};
      enemyJson["size"] = {enemy.GetSprite().GetOriginalSize().x,
                           enemy.GetSprite().GetOriginalSize().y};
      enemyJson["scale"] = {enemy.GetSprite().GetScale().x, enemy.GetSprite().GetScale().y};
      enemyJson["rotation"] = enemy.GetSprite().GetRotation();
      enemyJson["texture"] = enemy.GetSprite().GetTextureName();
      enemyJson["render_layer"] = enemy.GetSprite().GetRenderInfo().layer;

      enemyJson["animation type"] =
         enemy.GetAnimationType() == Animatable::ANIMATION_TYPE::LOOP ? "Loop" : "Reversable";

      const auto keypoints = enemy.GetAnimationKeypoints();
      for (const auto& point : keypoints)
      {
         nlohmann::json animationPoint;
         animationPoint["end position"] = {point.m_end.x, point.m_end.y};
         animationPoint["time duration"] = point.m_timeDuration.count();

         enemyJson["animate positions"].emplace_back(animationPoint);
      }

      json["ENEMIES"].emplace_back(enemyJson);
   }

   // OBJECTS
   for (const auto& object : objects_)
   {
      nlohmann::json objectJson;

      objectJson["name"] = object.GetName();
      objectJson["has collision"] = object.GetHasCollision();

      objectJson["position"] = {object.GetPosition().x, object.GetPosition().y};
      objectJson["size"] = {object.GetSprite().GetOriginalSize().x,
                            object.GetSprite().GetOriginalSize().y};
      objectJson["scale"] = {object.GetSprite().GetScale().x, object.GetSprite().GetScale().y};
      objectJson["rotation"] = object.GetSprite().GetRotation();
      objectJson["texture"] = object.GetSprite().GetTextureName();
      objectJson["render_layer"] = object.GetSprite().GetRenderInfo().layer;

      json["OBJECTS"].emplace_back(objectJson);
   }

   FileManager::SaveJsonFile(pathToLevel, json);
}

void
Level::Quit()
{
   objects_.clear();
   enemies_.clear();
}

void
Level::LockCamera()
{
   m_locked = true;
}


void
Level::UnlockCamera()
{
   m_locked = false;
}

bool
Level::IsCameraLocked() const
{
   return m_locked;
}

glm::vec2
Level::GetLevelPosition() const
{
   return m_background.GetPosition();
}

glm::ivec2
Level::GetSize() const
{
   return m_levelSize;
}

uint32_t
Level::GetTileSize() const
{
   return m_tileWidth;
}

PathFinder&
Level::GetPathfinder()
{
   return m_pathFinder;
}

const Player&
Level::GetPlayer() const
{
   return player_;
}

Object::ID
Level::AddGameObject(ObjectType objectType, const glm::vec2& position)
{
   const auto defaultSize = glm::ivec2(128, 128);
   const auto defaultTexture = std::string("Default128.png");

   Object::ID newObject = Object::INVALID_ID;

   switch (objectType)
   {
      case ObjectType::ENEMY: {
         const auto& newEnemy =
            enemies_.emplace_back(m_contextPointer, position, defaultSize, defaultTexture,
                                  std::vector< AnimationPoint >{});

         newObject = newEnemy.GetID();
      }
      break;

      case ObjectType::PLAYER: {
         if (player_.GetID() == Object::INVALID_ID)
         {
            player_.Setup(m_contextPointer, glm::vec3{position, 0.0f}, defaultSize, defaultTexture);
         }
         newObject = player_.GetID();
      }
      break;

      case ObjectType::OBJECT: {
         const auto& newObj = objects_.emplace_back(m_contextPointer, position, defaultSize,
                                                    defaultTexture, ObjectType::OBJECT);
         newObject = newObj.GetID();
         objectToIdx_[newObject] = objects_.size() - 1;
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
                       const std::vector< Tile >& currentTiles, Object::ID objectID,
                       bool hasCollision)
{
   auto newTiles = GetTilesFromBoundingBox(box);

   if (m_pathFinder.IsInitialized())
   {
      // TODO: Discard common tiles and only free/occupy unique ones
      FreeNodes(objectID, currentTiles, hasCollision);
      OccupyNodes(objectID, newTiles, hasCollision);
   }

   return newTiles;
}

void
Level::FreeNodes(Object::ID object, const std::vector< Tile >& nodes, bool hasCollision)
{
   if (m_pathFinder.IsInitialized())
   {
      for (auto tileID : nodes)
      {
         if (hasCollision)
         {
            m_pathFinder.SetNodeFreed(tileID, object);
         }

         m_pathFinder.SetObjectOffNode(tileID, object);
      }
   }
}

void
Level::OccupyNodes(Object::ID object, const std::vector< Tile >& nodes, bool hasCollision)
{
   if (m_pathFinder.IsInitialized())
   {
      for (auto tileID : nodes)
      {
         if (hasCollision)
         {
            m_pathFinder.SetNodeOccupied(tileID, object);
         }

         m_pathFinder.SetObjectOnNode(tileID, object);
      }
   }
}

std::vector< Tile >
Level::GetTilesFromBoundingBox(const std::array< glm::vec2, 4 >& box) const
{
   std::set< Tile > ret;

   auto insertToVec = [&ret](const std::vector< Tile >& tiles) {
      for (const auto& tile : tiles)
      {
         ret.insert(tile);
      }
   };

   // Standard Rect
   insertToVec(GetTilesAlongTheLine(box[0], box[3]));
   insertToVec(GetTilesAlongTheLine(box[3], box[2]));
   insertToVec(GetTilesAlongTheLine(box[2], box[1]));
   insertToVec(GetTilesAlongTheLine(box[1], box[0]));

   // Diagonals for inside nodes
   insertToVec(GetTilesAlongTheLine(box[0], box[2]));
   insertToVec(GetTilesAlongTheLine(box[1], box[3]));

   return std::vector< Tile >{ret.begin(), ret.end()};
}

std::vector< Tile >
Level::GetTilesFromRectangle(const std::array< glm::vec2, 4 >& rect) const
{
   std::vector< Tile > tiles;

   auto CustomGetTileFromPosition = [](const glm::ivec2& levelSize, uint32_t tileWidth,
                                       const glm::vec2& position) -> Tile {
      const auto w =
         static_cast< int32_t >(glm::floor(position.x / static_cast< float >(tileWidth)));
      const auto h =
         static_cast< int32_t >(glm::floor(position.y / static_cast< float >(tileWidth)));

      return {glm::clamp(w, 0, (levelSize.x / static_cast< int32_t >(tileWidth)) - 1),
              glm::clamp(h, 0, (levelSize.y / static_cast< int32_t >(tileWidth))) - 1};
   };

   // 'rect' is not rotated so we can assume left/right top/down boundaries
   const std::array< Tile, 4 > tileRect = {
      CustomGetTileFromPosition(m_levelSize, m_tileWidth, rect.at(0)),
      CustomGetTileFromPosition(m_levelSize, m_tileWidth, rect.at(1)),
      CustomGetTileFromPosition(m_levelSize, m_tileWidth, rect.at(2)),
      CustomGetTileFromPosition(m_levelSize, m_tileWidth, rect.at(3))};

   // 'y' tiles go bottom to top
   for (auto y = tileRect.at(2).second; y <= tileRect.at(0).second; ++y)
   {
      for (auto x = tileRect.at(0).first; x <= tileRect.at(1).first; ++x)
      {
         if (x != -1 and y != -1)
         {
            tiles.emplace_back(x, y);
         }
      }
   }

   return tiles;
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
   return position.x >= 0 && position.x < static_cast< float >(m_levelSize.x) && position.y >= 0
          && position.y < static_cast< float >(m_levelSize.y);
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

   // NOLINTNEXTLINE
   const auto numSteps = static_cast< int32_t >(glm::length(toPos - fromPos)
                                                / static_cast< float >(m_tileWidth / uint32_t{2}));

   const auto stepSize = (toPos - fromPos) / static_cast< float >(numSteps);

   for (int32_t i = 0; i < numSteps; ++i)
   {
      tiles.insert(GetTileFromPosition(fromPos + (stepSize * static_cast< float >(i))));
   }

   return {tiles.begin(), tiles.end()};
}

void
Level::GenerateTextureForCollision()
{
   const auto width = static_cast< size_t >(m_levelSize.x) / static_cast< size_t >(m_tileWidth);
   const auto height = static_cast< size_t >(m_levelSize.y) / static_cast< size_t >(m_tileWidth);
   const size_t numChannels = 4;
   const auto size = static_cast< size_t >(width * height * numChannels);

   auto data = FileManager::ImageHandleType{new unsigned char[size],
                                            [](const uint8_t* ptr) { delete[] ptr; }};
   const auto& nodes = m_pathFinder.GetAllNodes();

   for (size_t h = 0; h < height; ++h)
   {
      const auto offset = height - 1 - (h % height);
      for (size_t w = 0; w < width; ++w)
      {
         const auto occupied = nodes.at(static_cast< size_t >(w + width * h)).occupied_;
         const auto index =
            (w + width * offset) * numChannels; // Calculate the index for the start of this pixel

         data[index + 0] = 255;             // R
         data[index + 1] = !occupied * 255; // G
         data[index + 2] = !occupied * 255; // B
         data[index + 3] = 255;             // A
      }
   }

   collisionTextureData_ = {std::move(data), {width, height}, numChannels};

   // To avoid blurry edges
   renderer::TextureProperties props;
   props.magFilter = VK_FILTER_NEAREST;
   props.minFilter = VK_FILTER_NEAREST;

   const auto* texture = renderer::TextureLibrary::CreateTexture(
      renderer::TextureType::DIFFUSE_MAP, "Collision", collisionTextureData_, props);

   collisionTexture_ = texture->GetID();
}

void
Level::LoadPremade(const std::string& fileName, const glm::ivec2& size)
{
   m_locked = false;
   m_levelSize = size;

   m_background.SetSpriteTextured(glm::vec3(static_cast< float >(m_levelSize.x) / 2.0f,
                                            static_cast< float >(m_levelSize.y) / 2.0f, 0.5f),
                                  size, fileName);

   baseTexture_ = m_background.GetTexture()->GetID();
}

void
Level::LoadShaders(const std::string& /*shaderName*/)
{
   // m_shaders.LoadShaders(shaderName);
}

void
Level::DeleteObject(Object::ID deletedObject)
{
   switch (Object::GetTypeFromID(deletedObject))
   {
      case ObjectType::PLAYER: {
      }
      break;
      case ObjectType::ENEMY: {
         auto enemyIter = stl::find_if(enemies_, [deletedObject](const auto& enemy) {
            return enemy.GetID() == deletedObject;
         });

         utils::Assert(
            enemyIter != enemies_.end(),
            fmt::format("Level: Trying to delete an Enemy that doesn't exist! Object ID: {}",
                        deletedObject));

         enemyIter->GetSprite().ClearData();
         // Don't call erase
         std::iter_swap(enemyIter, enemies_.end() - 1);
         enemies_.pop_back();
      }
      break;
      case ObjectType::OBJECT: {
         auto objectIter = stl::find_if(objects_, [deletedObject](const auto& object) {
            return object.GetID() == deletedObject;
         });

         utils::Assert(
            objectIter != objects_.end(),
            fmt::format("Level: Trying to delete an object that doesn't exist! Object type: {}",
                        Object::GetTypeString(deletedObject)));

         objectIter->GetSprite().ClearData();

         if (objectIter != objects_.end() - 1)
         {
            const auto deletedIdx = objectToIdx_.at(deletedObject);
            objectToIdx_.erase(deletedObject);
            objectToIdx_.at(objects_.back().GetID()) = deletedIdx;

            // Don't call erase
            std::iter_swap(objectIter, objects_.end() - 1);
         }

         objects_.pop_back();
      }
      break;
      default: {
      }
   }
}

Object&
Level::GetObjectRef(Object::ID objectID)
{
   Object* requestedObject = nullptr;

   switch (Object::GetTypeFromID(objectID))
   {
      case ObjectType::OBJECT: {
         const auto idx = objectToIdx_.at(objectID);

         requestedObject = &objects_.at(idx);
      }
      break;
      case ObjectType::ENEMY: {
         auto enemyIt = stl::find_if(
            enemies_, [objectID](const auto& enemy) { return enemy.GetID() == objectID; });

         utils::Assert(enemyIt != enemies_.end(),
                       fmt::format("Object with Type={} and ID={} not found!",
                                   Object::GetTypeString(objectID), objectID));

         requestedObject = &(*enemyIt);
      }
      break;

      case ObjectType::PLAYER: {
         requestedObject = &player_;
      }
      break;

      case ObjectType::ANIMATION_POINT: {
         for (auto& enemy : enemies_)
         {
            auto& points = enemy.GetAnimationKeypoints();
            auto it = stl::find_if(
               points, [objectID](const auto& point) { return point.GetID() == objectID; });

            if (it != points.end())
            {
               requestedObject = &(*it);
               break;
            }
         }
      }
      break;

      case ObjectType::PATHFINDER_NODE: {
         auto& nodes = m_pathFinder.GetAllNodes();
         auto it =
            stl::find_if(nodes, [objectID](const auto& node) { return node.GetID() == objectID; });

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

   // requestedObject will never be nullptr
   // NOLINTNEXTLINE
   return *requestedObject;
}

GameObject&
Level::GetGameObjectRef(Object::ID gameObjectID)
{
   const auto type = Object::GetTypeFromID(gameObjectID);

   GameObject* requestedObject = nullptr;

   switch (type)
   {
      case ObjectType::OBJECT: {
         const auto idx = objectToIdx_.at(gameObjectID);

         requestedObject = &objects_.at(idx);
      }
      break;
      case ObjectType::ENEMY: {
         auto enemyIt = stl::find_if(
            enemies_, [gameObjectID](const auto& enemy) { return enemy.GetID() == gameObjectID; });

         utils::Assert(enemyIt != enemies_.end(),
                       fmt::format("Object with Type={} and ID={} not found!",
                                   Object::GetTypeString(gameObjectID), gameObjectID));

         requestedObject = &(*enemyIt);
      }
      break;

      case ObjectType::PLAYER: {
         requestedObject = &player_;
      }
      break;

      default: {
         Logger::Fatal("Level::GetGameObjectRef called with invalid type {}!\n",
                       Object::GetTypeString(gameObjectID));
      }
   }

   // This should never happen
   // NOLINTNEXTLINE
   assert(requestedObject);

   // requestedObject will never be nullptr
   // NOLINTNEXTLINE
   return *requestedObject;
}

void
Level::Update(bool isReverse)
{
   m_background.Update(isReverse);

   // TODO: Parallelize for larger groups of objects
   // Player and Enemies should be handled by a single thread,
   // since they're dependent of eachother
   for (auto& obj : objects_)
   {
      // We probably should update only objects that are
      // in some range of camera
      if (obj.Visible())
      {
         obj.Update(isReverse);
      }
   }

   for (auto& enemy : enemies_)
   {
      if (enemy.Visible())
      {
         enemy.Update(isReverse);
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
   for (auto& obj : objects_)
   {
      if (obj.Visible())
      {
         obj.Render();
      }
   }

   for (auto& enemy : enemies_)
   {
      if (enemy.Visible())
      {
         enemy.Render();
      }
   }

   player_.Render();
}

void
Level::MoveObjs(const glm::vec2& moveBy)
{
   for (auto& obj : objects_)
   {
      obj.Move(moveBy);
   }

   for (auto& enemy : enemies_)
   {
      enemy.Move(moveBy);
   }

   player_.Move(moveBy);
}

const std::vector< GameObject >&
Level::GetObjects() const
{
   return objects_;
}

const std::vector< Enemy >&
Level::GetEnemies() const
{
   return enemies_;
}

void
Level::SetPlayersPosition(const glm::vec2& /*position*/)
{
   // m_playerPos = position;
   // m_playerPos /= m_tileSize;
}

Object::ID
Level::GetGameObjectOnLocation(const glm::vec2& screenPosition)
{
   Object::ID object = Object::INVALID_ID;
   const auto globalPos = m_contextPointer->ScreenToGlobal(screenPosition);

   if (IsInLevelBoundaries(globalPos))
   {
      const auto& node = m_pathFinder.GetNodeFromPosition(globalPos);

      // Try luck with Tile at mouse pos
      auto objectOnLocation =
         stl::find_if(node.objectsOnThisNode_, [this, screenPosition](const auto& objectID) {
            const auto& object = GetGameObjectRef(objectID);
            return object.CheckIfCollidedScreenPosion(screenPosition);
         });

      if (objectOnLocation != node.objectsOnThisNode_.end())
      {
         object = *objectOnLocation;
      }
      // If Tile at mouse pos doesn't have any Objects on it
      // See neighbouring Tiles, because of our collision generation,
      // we only generate outline and diagonals
      else
      {
         const int32_t xLower = glm::max(0, node.tile_.first - 2);
         const int32_t xUpper = glm::min(127, node.tile_.first + 2);

         const int32_t yLower = glm::max(0, node.tile_.second - 2);
         const int32_t yUpper = glm::min(127, node.tile_.second + 2);
         for (int32_t y = yLower; y < yUpper; ++y)
         {
            for (int32_t x = xLower; x < xUpper; ++x)
            {
               const auto& neighbour = m_pathFinder.GetNodeFromTile({x, y});
               auto objectFound = stl::find_if(
                  neighbour.objectsOnThisNode_, [this, screenPosition](const auto& objectID) {
                     const auto& object = GetGameObjectRef(objectID);
                     return object.CheckIfCollidedScreenPosion(screenPosition);
                  });

               if (objectFound != neighbour.objectsOnThisNode_.end())
               {
                  object = *objectFound;
                  break;
               }
            }
         }
      }
   }

   return object;
}

Object::ID
Level::GetGameObjectOnLocationAndLayer(const glm::vec2& screenPosition, int32_t renderLayer)
{
   Object::ID object = Object::INVALID_ID;
   const auto globalPos = m_contextPointer->ScreenToGlobal(screenPosition);

   if (IsInLevelBoundaries(globalPos))
   {
      if (renderLayer != -1)
      {
         const auto& node = m_pathFinder.GetNodeFromPosition(globalPos);

         auto objectOnLocation = stl::find_if(
            node.objectsOnThisNode_, [this, screenPosition, renderLayer](const auto& objectID) {
               const auto& object = GetGameObjectRef(objectID);
               return object.CheckIfCollidedScreenPosion(screenPosition)
                      and (object.GetSprite().GetRenderInfo().layer == renderLayer);
            });

         if (objectOnLocation != node.objectsOnThisNode_.end())
         {
            object = *objectOnLocation;
         }
      }
      else
      {
         object = GetGameObjectOnLocation(screenPosition);
      }
   }

   return object;
}

void
Level::RenderPathfinder(bool render)
{
   render ? m_background.SetTextureID(renderer::TextureType::MASK_MAP, collisionTexture_)
          : m_background.SetTextureID(renderer::TextureType::MASK_MAP, baseTexture_);
}

void
Level::UpdateCollisionTexture()
{
   const auto& tilesChanged = m_pathFinder.GetNodesModifiedLastFrame();
   auto* data = collisionTextureData_.m_bytes.get();
   const auto tileWidth = static_cast< int32_t >(m_tileWidth);
   const auto width = m_levelSize.x / tileWidth;

   if (!tilesChanged.empty())
   {
      for (const auto& tile : tilesChanged)
      {
         const auto& node = m_pathFinder.GetNodeFromTile(tile);
         const auto x = tile.first;
         const auto y = tile.second;
         const auto offset = tileWidth - 1 - (y % tileWidth);

         const auto index = (x + width * offset) * 4;

         data[index + 0] = 255;                   // R
         data[index + 1] = !node.occupied_ * 255; // G
         data[index + 2] = !node.occupied_ * 255; // B
         data[index + 3] = 255;                   // A
      }

      renderer::TextureLibrary::GetTexture(collisionTexture_)->UpdateTexture(collisionTextureData_);
   }
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
