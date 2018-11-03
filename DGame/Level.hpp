#pragma once

#include <PathFinder.hpp>
#include <Player.hpp>
#include <Shaders.hpp>
#include <glm.hpp>
#include <unordered_map>

class GameObject;

class Level
{
   // IN PROGRESS

   Sprite background;
   std::unordered_map< std::string, Texture > textures;
   Shaders shaders{};

   glm::vec2 cameraPosition;
   glm::ivec2 cameraTilePos;

   glm::ivec2 playerPos;

   bool locked{};

   glm::ivec2 tileSize;
   glm::ivec2 numTuilesOnScreen;
   glm::ivec2 tilesToDraw;

   glm::ivec2 levelSize;
   std::vector< std::unique_ptr< GameObject > > objects;
   PathFinder m_pathinder{};

 public:
   Level() = default;
   ~Level() = default;

   // Convert from OpenGL position to map position
   glm::vec2
   GetLocalVec(const glm::vec2& local) const;

   // Convert from map position to OpenGL
   glm::vec2
   GetGlobalVec(const glm::vec2& local) const;

   void
   MoveObjs(const glm::vec2& moveBy, bool isCameraMovement = true);
   void
   Load(const std::string& fileName);
   void
   LoadPremade(const std::string& fileName, const glm::ivec2& size);
   void
   LoadShaders(const std::string& shaderName);
   void
   AddGameObject(const glm::vec2& pos, const glm::ivec2& size, const std::string& sprite);
   void
   Move(const glm::vec2& moveBy);
   void
   Draw();

   bool
   CheckCollision(const glm::ivec2& localPos, const Player& player);
   void
   LockCamera()
   {
      locked = true;
   }
   void
   UnlockCamera()
   {
      locked = false;
   }
   bool
   IsCameraLocked() const
   {
      return locked;
   }
   glm::vec2
   GetCameraPosition()
   {
      return cameraPosition;
   }
   glm::ivec2
   GetCameraTiledPosition()
   {
      return cameraTilePos;
   }
   glm::vec2
   GetLevelPosition() const
   {
      return background.GetPosition();
   }
   glm::ivec2
   GetSize() const
   {
      return levelSize;
   }
   const PathFinder&
   GetPathfinder() const
   {
      return m_pathinder;
   }

   void
   SetPlayersPosition(const glm::vec2& position);
   void
   MoveCamera(const glm::vec2& moveBy);

   glm::ivec2
   CheckMoveCamera(const glm::vec2& moveBy) const;
   glm::ivec2
   GetTilePosition(const glm::vec2& position) const;
};
