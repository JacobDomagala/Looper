#include "level.hpp"
#include "enemy.hpp"
#include "game.hpp"
#include "player.hpp"
#include "renderer/lighting_json.hpp"
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
   SetLightingSettings({});
   name_ = name;
   levelSize_ = size;

   background_.SetLightingMaterial({
      .emissionColor = glm::vec3(1.0f),
      .emissionStrength = 0.0f,
      .opacity = 0.0f,
      .receivesLight = true,
   });
   background_.SetSpriteTextured(glm::vec3(static_cast< float >(levelSize_.x) / 2.0f,
                                           static_cast< float >(levelSize_.y) / 2.0f,
                                           renderer::LAYER_10),
                                 size, "white.png");

   contextPointer_ = context;
   pathFinder_.Initialize(this);
}

void
Level::Load(Application* context, const std::string& pathToLevel)
{
   const auto json = FileManager::LoadJsonFile(pathToLevel);
   SetLightingSettings(
      renderer::ReadLightingSettings(json.value("LIGHTING", nlohmann::json::object())));

   // BACKGROUND
   {
      const auto& background = json.at("BACKGROUND");
      const auto& backgroundTex = background.at("texture");
      const auto& size = background.at("size");

      LoadPremade(backgroundTex, glm::ivec2(size.at(0), size.at(1)));

      contextPointer_ = context;
   }

   // PATHFINDER
   {
      SCOPED_TIMER(fmt::format("Loading Pathfinder"));
      pathFinder_.Initialize(this);
   }

   // PLAYER
   {
      SCOPED_TIMER("Loading Player");

      const auto& player = json.at("PLAYER");
      const auto& position = player.at("position");
      const auto& size = player.at("size");
      const auto& texture = player.at("texture");
      // const auto weapons = json[key]["weapons"];
      const auto& name = player.at("name");

      player_.Setup(context, glm::vec2{position.at(0), position.at(1)},
                    glm::ivec2(size.at(0), size.at(1)), texture, name);
      player_.Rotate(player.at("rotation"));
      player_.editorGroup_ = player.at("editor_group");
      player_.GetSprite().SetLightingMaterial(
         renderer::ReadLightingMaterial(player.value("lighting", nlohmann::json::object()),
                                        player_.GetSprite().GetLightingMaterial()));
   }

   // ENEMIES
   {
      const auto& enemies = json.at("ENEMIES");
      SCOPED_TIMER(fmt::format("Loading Enemies ({})", enemies.size()));
      // This is a magic number that we should figure out later
      enemies_.reserve(100);
      enemies_.resize(enemies.size());

      for (size_t i = 0; i < enemies.size(); ++i)
      {
         const auto& enemy = enemies.at(i);
         const auto& position = enemy.at("position");
         const auto& size = enemy.at("size");
         const auto& texture = enemy.at("texture");
         const auto& name = enemy.at("name");

         auto& object = enemies_.at(i);
         object.Setup(context, glm::vec2{position.at(0), position.at(1)},
                      glm::ivec2(size.at(0), size.at(1)), texture, std::vector< AnimationPoint >{});
         object.SetName(name);
         object.Rotate(enemy.at("rotation"));

         std::vector< AnimationPoint > keypointsPositions = {};
         glm::vec2 beginPoint = glm::vec2(position.at(0), position.at(1));

         for (const auto& point : enemy.at("animate positions"))
         {
            AnimationPoint animationPoint(object.GetID());
            animationPoint.m_end =
               glm::vec2(point.at("end position").at(0), point.at("end position").at(1));
            animationPoint.m_timeDuration = time::seconds(point.at("time duration"));

            keypointsPositions.emplace_back(animationPoint);

            beginPoint = animationPoint.m_end;
         }

         object.SetAnimationKeypoints(std::move(keypointsPositions));
         object.editorGroup_ = enemy.at("editor_group");
         object.GetSprite().SetLightingMaterial(
            renderer::ReadLightingMaterial(enemy.value("lighting", nlohmann::json::object()),
                                           object.GetSprite().GetLightingMaterial()));
      }
   }

   // OBJECTS
   {
      const auto& objects = json.at("OBJECTS");
      SCOPED_TIMER(fmt::format("Loading Objects ({})", objects.size()));
      // This is a magic number that we should figure out later
      objects_.reserve(10000);
      objects_.resize(objects.size());
      for (size_t i = 0; i < objects.size(); ++i)
      {
         const auto& object = objects.at(i);
         const auto& position = object.at("position");
         const auto& size = object.at("size");
         const auto& texture = object.at("texture");
         const auto& name = object.at("name");

         auto& gameObject = objects_.at(i);
         gameObject.Setup(context, glm::vec2{position.at(0), position.at(1)},
                          glm::ivec2(size.at(0), size.at(1)), texture, ObjectType::OBJECT,
                          object.at("render_layer"));
         objectToIdx_[gameObject.GetID()] = i;
         gameObject.SetName(name);
         gameObject.Rotate(object.at("rotation"));
         gameObject.SetHasCollision(object.at("has collision"));
         gameObject.editorGroup_ = object.at("editor_group");
         gameObject.GetSprite().SetLightingMaterial(
            renderer::ReadLightingMaterial(object.value("lighting", nlohmann::json::object()),
                                           gameObject.GetSprite().GetLightingMaterial()));
      }
   }
}

void
Level::Save(const std::string& pathToLevel)
{
   nlohmann::json json = {
      {"BACKGROUND", nlohmann::json::object()},
      {"PLAYER", nlohmann::json::object()},
   };
   json.emplace("LIGHTING", renderer::WriteLightingSettings(lighting_));

   // Serialize background
   json.at("BACKGROUND").emplace("texture", background_.GetTextureName());
   json.at("BACKGROUND")
      .emplace("size", nlohmann::json{background_.GetSize().x, background_.GetSize().y});

   // PLAYER
   json.at("PLAYER").emplace("name", player_.GetName());
   json.at("PLAYER").emplace("position",
                             nlohmann::json{player_.GetPosition().x, player_.GetPosition().y});
   json.at("PLAYER").emplace("rotation", player_.GetSprite().GetRotation());
   json.at("PLAYER").emplace("size", nlohmann::json{
                                        player_.GetSprite().GetOriginalSize().x,
                                        player_.GetSprite().GetOriginalSize().y,
                                     });
   json.at("PLAYER").emplace("texture", player_.GetSprite().GetTextureName());
   json.at("PLAYER").emplace("weapons", player_.GetWeapons());
   json.at("PLAYER").emplace("render_layer", player_.GetSprite().GetRenderInfo().layer);
   json.at("PLAYER").emplace("editor_group", player_.editorGroup_);
   json.at("PLAYER").emplace(
      "lighting", renderer::WriteLightingMaterial(player_.GetSprite().GetLightingMaterial()));


   // ENEMIES
   if (!enemies_.empty())
   {
      json.emplace("ENEMIES", nlohmann::json::array());
   }
   for (const auto& enemy : enemies_)
   {
      nlohmann::json enemyJson;

      enemyJson.emplace("name", enemy.GetName());
      enemyJson.emplace("position", nlohmann::json{enemy.GetPosition().x, enemy.GetPosition().y});
      enemyJson.emplace("size", nlohmann::json{
                                   enemy.GetSprite().GetOriginalSize().x,
                                   enemy.GetSprite().GetOriginalSize().y,
                                });
      enemyJson.emplace("rotation", enemy.GetSprite().GetRotation());
      enemyJson.emplace("texture", enemy.GetSprite().GetTextureName());
      enemyJson.emplace("render_layer", enemy.GetSprite().GetRenderInfo().layer);
      enemyJson.emplace("editor_group", enemy.editorGroup_);
      enemyJson.emplace("lighting",
                        renderer::WriteLightingMaterial(enemy.GetSprite().GetLightingMaterial()));

      enemyJson.emplace(
         "animation type",
         enemy.GetAnimationType() == Animatable::ANIMATION_TYPE::LOOP ? "Loop" : "Reversable");

      const auto keypoints = enemy.GetAnimationKeypoints();
      if (!keypoints.empty())
      {
         enemyJson.emplace("animate positions", nlohmann::json::array());
      }
      for (const auto& point : keypoints)
      {
         nlohmann::json animationPoint;
         animationPoint.emplace("end position", nlohmann::json{point.m_end.x, point.m_end.y});
         animationPoint.emplace("time duration", point.m_timeDuration.count());

         enemyJson.at("animate positions").emplace_back(animationPoint);
      }

      json.at("ENEMIES").emplace_back(enemyJson);
   }

   // OBJECTS
   if (!objects_.empty())
   {
      json.emplace("OBJECTS", nlohmann::json::array());
   }
   for (const auto& object : objects_)
   {
      nlohmann::json objectJson;

      objectJson.emplace("name", object.GetName());
      objectJson.emplace("has collision", object.GetHasCollision());

      objectJson.emplace("position",
                         nlohmann::json{object.GetPosition().x, object.GetPosition().y});
      objectJson.emplace("size", nlohmann::json{
                                    object.GetSprite().GetOriginalSize().x,
                                    object.GetSprite().GetOriginalSize().y,
                                 });
      objectJson.emplace("rotation", object.GetSprite().GetRotation());
      objectJson.emplace("texture", object.GetSprite().GetTextureName());
      objectJson.emplace("render_layer", object.GetSprite().GetRenderInfo().layer);
      objectJson.emplace("editor_group", object.editorGroup_);
      objectJson.emplace("lighting",
                         renderer::WriteLightingMaterial(object.GetSprite().GetLightingMaterial()));

      json.at("OBJECTS").emplace_back(objectJson);
   }

   FileManager::SaveJsonFile(pathToLevel, json);
}

const renderer::LightingSettings&
Level::GetLightingSettings() const
{
   return lighting_;
}

void
Level::SetLightingSettings(const renderer::LightingSettings& settings)
{
   lighting_ = renderer::SanitizeLighting(settings);
   renderer::GetRenderData().lighting = lighting_;
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
   locked_ = true;
}


void
Level::UnlockCamera()
{
   locked_ = false;
}

bool
Level::IsCameraLocked() const
{
   return locked_;
}

glm::vec2
Level::GetLevelPosition() const
{
   return background_.GetPosition();
}

glm::ivec2
Level::GetSize() const
{
   return levelSize_;
}

uint32_t
Level::GetTileSize() const
{
   return tileWidth_;
}

size_t
Level::GetNumOfObjects() const
{
   return objects_.size() + enemies_.size() + 1 /* player */;
}

PathFinder&
Level::GetPathfinder()
{
   return pathFinder_;
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
            enemies_.emplace_back(contextPointer_, position, defaultSize, defaultTexture,
                                  std::vector< AnimationPoint >{});

         newObject = newEnemy.GetID();
      }
      break;

      case ObjectType::PLAYER: {
         if (player_.GetID() == Object::INVALID_ID)
         {
            player_.Setup(contextPointer_, glm::vec3{position, 0.0f}, defaultSize, defaultTexture);
         }
         newObject = player_.GetID();
      }
      break;

      case ObjectType::OBJECT: {
         const auto& newObj = objects_.emplace_back(contextPointer_, position, defaultSize,
                                                    defaultTexture, ObjectType::OBJECT);
         newObject = newObj.GetID();
         objectToIdx_[newObject] = objects_.size() - 1;
      }
      break;

      default: {
      }
   }

   return newObject;
}

std::vector< Tile >
Level::GameObjectMoved(const std::array< glm::vec2, 4 >& box,
                       const std::vector< Tile >& currentTiles, Object::ID objectID,
                       bool hasCollision)
{
   auto newTiles = GetTilesFromBoundingBox(box);

   if (pathFinder_.IsInitialized())
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
   if (pathFinder_.IsInitialized())
   {
      for (auto const tileID : nodes)
      {
         if (hasCollision)
         {
            pathFinder_.SetNodeFreed(tileID, object);
         }

         pathFinder_.SetObjectOffNode(tileID, object);
      }
   }
}

void
Level::OccupyNodes(Object::ID object, const std::vector< Tile >& nodes, bool hasCollision)
{
   if (pathFinder_.IsInitialized())
   {
      for (auto const tileID : nodes)
      {
         if (hasCollision)
         {
            pathFinder_.SetNodeOccupied(tileID, object);
         }

         pathFinder_.SetObjectOnNode(tileID, object);
      }
   }
}

std::vector< Tile >
Level::GetTilesFromBoundingBox(const std::array< glm::vec2, 4 >& box) const
{
   std::set< Tile > ret;

   auto const insertToVec = [&ret](const std::vector< Tile >& tiles) {
      ret.insert(tiles.begin(), tiles.end());
   };

   // Standard Rect
   insertToVec(GetTilesAlongTheLine(box.at(0), box.at(3)));
   insertToVec(GetTilesAlongTheLine(box.at(3), box.at(2)));
   insertToVec(GetTilesAlongTheLine(box.at(2), box.at(1)));
   insertToVec(GetTilesAlongTheLine(box.at(1), box.at(0)));

   // Diagonals for inside nodes
   insertToVec(GetTilesAlongTheLine(box.at(0), box.at(2)));
   insertToVec(GetTilesAlongTheLine(box.at(1), box.at(3)));

   return std::vector< Tile >{ret.begin(), ret.end()};
}

std::vector< Tile >
Level::GetTilesFromRectangle(const std::array< glm::vec2, 4 >& rect) const
{
   std::vector< Tile > tiles;

   auto const CustomGetTileFromPosition = [](const glm::ivec2& levelSize, uint32_t tileWidth,
                                             const glm::vec2& position) -> Tile {
      const auto w =
         static_cast< int32_t >(glm::floor(position.x / static_cast< float >(tileWidth)));
      const auto h =
         static_cast< int32_t >(glm::ceil(position.y / static_cast< float >(tileWidth)));

      return {glm::clamp(w, 0, (levelSize.x / static_cast< int32_t >(tileWidth)) - 1),
              glm::clamp(h, 0, (levelSize.y / static_cast< int32_t >(tileWidth))) - 1};
   };

   // 'rect' is not rotated so we can assume left/right top/down boundaries
   const std::array< Tile, 4 > tileRect = {
      CustomGetTileFromPosition(levelSize_, tileWidth_, rect.at(0)),
      CustomGetTileFromPosition(levelSize_, tileWidth_, rect.at(1)),
      CustomGetTileFromPosition(levelSize_, tileWidth_, rect.at(2)),
      CustomGetTileFromPosition(levelSize_, tileWidth_, rect.at(3)),
   };

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

   const auto w = static_cast< int32_t >(glm::floor(local.x / static_cast< float >(tileWidth_)));
   const auto h = static_cast< int32_t >(glm::floor(local.y / static_cast< float >(tileWidth_)));

   return {w, h};
}

bool
Level::IsInLevelBoundaries(const glm::vec2& position) const
{
   return position.x >= 0 && position.x < static_cast< float >(levelSize_.x) && position.y >= 0
          && position.y < static_cast< float >(levelSize_.y);
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
      auto const curPos = fromPos + (stepSize * static_cast< float >(i));

      if (!IsInLevelBoundaries(curPos) or pathFinder_.GetNodeFromPosition(curPos).occupied_)
      {
         break;
      }

      returnPosition = curPos;
   }

   return glm::clamp(returnPosition, glm::vec2{0.0f, 0.0f},
                     static_cast< glm::vec2 >(levelSize_ - 1));
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
      if (!IsInLevelBoundaries(curPos) or pathFinder_.GetNodeFromPosition(curPos).occupied_)
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

   const auto numSteps =
      static_cast< uint32_t >(glm::length(toPos - fromPos)) / (tileWidth_ / uint32_t{2});

   const auto stepSize = (toPos - fromPos) / static_cast< float >(numSteps);

   for (uint32_t i = 0; i < numSteps; ++i)
   {
      tiles.insert(GetTileFromPosition(fromPos + (stepSize * static_cast< float >(i))));
   }

   return {tiles.begin(), tiles.end()};
}

void
Level::GenerateTextureForCollision()
{
   const auto width = static_cast< size_t >(levelSize_.x) / static_cast< size_t >(tileWidth_);
   const auto height = static_cast< size_t >(levelSize_.y) / static_cast< size_t >(tileWidth_);
   const size_t numChannels = 4;
   const auto size = (width * height * numChannels);

   auto data = FileManager::ImageHandleType{new unsigned char[size],
                                            [](const uint8_t* ptr) { delete[] ptr; }};
   const auto& nodes = pathFinder_.GetAllNodes();

   for (size_t h = 0; h < height; ++h)
   {
      const auto offset = height - 1 - (h % height);
      for (size_t w = 0; w < width; ++w)
      {
         const auto occupied = nodes.at(w + (width * h)).occupied_;
         const auto index =
            (w + (width * offset)) * numChannels; // Calculate the index for the start of this pixel

         auto* pixel = data.get() + index;
         pixel[0] = 255;             // R
         pixel[1] = !occupied * 255; // G
         pixel[2] = !occupied * 255; // B
         pixel[3] = 255;             // A
      }
   }

   collisionTextureData_ = {
      .m_bytes = std::move(data),
      .m_size = {width, height},
      .m_format = numChannels,
   };

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
   locked_ = false;
   levelSize_ = size;

   background_.SetLightingMaterial({
      .emissionColor = glm::vec3(1.0f),
      .emissionStrength = 0.0f,
      .opacity = 0.0f,
      .receivesLight = true,
   });
   background_.SetSpriteTextured(glm::vec2(static_cast< float >(levelSize_.x) / 2.0f,
                                           static_cast< float >(levelSize_.y) / 2.0f),
                                 size, fileName, 10);

   baseTexture_ = background_.GetTexture()->GetID();
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
         auto const enemyIter = stl::find_if(enemies_, [deletedObject](const auto& enemy) {
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
         auto const objectIter = stl::find_if(objects_, [deletedObject](const auto& object) {
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
         auto const enemyIt = stl::find_if(
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
            auto const it = stl::find_if(
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
         auto& nodes = pathFinder_.GetAllNodes();
         auto const it =
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
         auto const enemyIt = stl::find_if(
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
   background_.Update(isReverse);

   player_.Update(isReverse);

   // TODO: Parallelize for larger groups of objects
   // Player and Enemies should be handled by a single thread,
   // since they're dependent of eachother
   // for (auto& obj : objects_)
   //{
   //   // We probably should update only objects that are
   //   // in some range of camera
   //   if (obj.Visible())
   //   {
   //      obj.Update(isReverse);
   //   }
   //}

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
   background_.Render();
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
Level::SetPlayersPosition([[maybe_unused]] const glm::vec2& position)
{
   // m_playerPos = position;
   // m_playerPos /= m_tileSize;
}

Object::ID
Level::GetGameObjectOnLocation(const glm::vec2& screenPosition)
{
   Object::ID object = Object::INVALID_ID;
   const auto globalPos = contextPointer_->ScreenToGlobal(screenPosition);

   if (IsInLevelBoundaries(globalPos))
   {
      const auto& node = pathFinder_.GetNodeFromPosition(globalPos);

      // Try luck with Tile at mouse pos
      auto const objectOnLocation =
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
               const auto& neighbour = pathFinder_.GetNodeFromTile({x, y});
               auto const objectFound = stl::find_if(
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
   const auto globalPos = contextPointer_->ScreenToGlobal(screenPosition);

   if (IsInLevelBoundaries(globalPos))
   {
      if (renderLayer != -1)
      {
         const auto& node = pathFinder_.GetNodeFromPosition(globalPos);

         auto const objectOnLocation = stl::find_if(
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
   render ? background_.SetTextureID(renderer::TextureType::MASK_MAP, collisionTexture_)
          : background_.SetTextureID(renderer::TextureType::MASK_MAP, baseTexture_);
}

void
Level::UpdateCollisionTexture()
{
   const auto& tilesChanged = pathFinder_.GetNodesModifiedLastFrame();
   auto* data = collisionTextureData_.m_bytes.get();
   const auto tileWidth = static_cast< int32_t >(tileWidth_);
   const auto width = levelSize_.x / tileWidth;

   if (!tilesChanged.empty())
   {
      for (const auto& tile : tilesChanged)
      {
         const auto& node = pathFinder_.GetNodeFromTile(tile);
         const auto x = tile.first;
         const auto y = tile.second;
         const auto offset = tileWidth - 1 - (y % tileWidth);

         const auto index = (x + (width * offset)) * 4;

         auto* pixel = data + index;
         pixel[0] = 255;                   // R
         pixel[1] = !node.occupied_ * 255; // G
         pixel[2] = !node.occupied_ * 255; // B
         pixel[3] = 255;                   // A
      }

      renderer::TextureLibrary::GetTexture(collisionTexture_)->UpdateTexture(collisionTextureData_);
   }
}

renderer::Sprite&
Level::GetSprite()
{
   return background_;
}

void
Level::SetSize(const glm::ivec2& newSize)
{
   const auto oldSize = static_cast< glm::vec2 >(levelSize_);
   levelSize_ = newSize;
   background_.SetSize(newSize);
   // Make sure top left is always 0,0
   const auto diff = oldSize / 2.0f - static_cast< glm::vec2 >(newSize) / 2.0f;

   background_.Translate(glm::vec3(-diff, 0.0f));
}

} // namespace looper
