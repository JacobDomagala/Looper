#pragma once

#include "PathFinder.hpp"
#include "Player.hpp"
#include "Shaders.hpp"

#include <glm/glm.hpp>
#include <unordered_map>

namespace dgame {

class Application;
class GameObject;
class Window;
class Game;

class Level
{
 public:
   Level() = default;

   ~Level() = default;

   std::shared_ptr< GameObject >
   AddGameObject(GameObject::TYPE objectType);

   // Convert from OpenGL position to map position
   glm::vec2
   GetLocalVec(const glm::vec2& local) const;

   // Convert from map position to OpenGL
   glm::vec2
   GetGlobalVec(const glm::vec2& local) const;

   void
   MoveObjs(const glm::vec2& moveBy, bool isCameraMovement = true);

   void
   Create(Application* context, const glm::ivec2& size);

   // pathToFile - global path to level file (.dgl)
   void
   Load(Application* context, const std::string& pathToFile);

   // pathToFile - global path to level file (.dgl)
   void
   Save(const std::string& pathToFile);

   void
   Quit();

   void
   LoadPremade(const std::string& fileName, const glm::ivec2& size);

   void
   LoadShaders(const std::string& shaderName);

   void
   AddGameObject(Game& game, const glm::vec2& pos, const glm::ivec2& size, const std::string& sprite);

   void
   Move(const glm::vec2& moveBy);

   void
   Scale(const glm::vec2& scaleVal);

   void
   Rotate(float angle, bool cumulative = false);

   void
   Update(bool isReverse);

   void
   Render();

   bool
   CheckCollision(const glm::ivec2& localPos, const Player& player);

   void
   LockCamera()
   {
      m_locked = true;
   }

   void
   UnlockCamera()
   {
      m_locked = false;
   }

   bool
   IsCameraLocked() const
   {
      return m_locked;
   }

   glm::vec2
   GetCameraPosition()
   {
      return m_cameraPosition;
   }

   glm::ivec2
   GetCameraTiledPosition()
   {
      return m_cameraTilePos;
   }

   glm::vec2
   GetLevelPosition() const
   {
      return m_background.GetPosition();
   }

   glm::ivec2
   GetSize() const
   {
      return m_levelSize;
   }

   const PathFinder&
   GetPathfinder() const
   {
      return m_pathinder;
   }

   std::shared_ptr< Player >
   GetPlayer()
   {
      return m_player;
   }

   std::vector< std::shared_ptr< GameObject > >&
   GetObjects();

   Shaders&
   GetShader()
   {
      return m_shaders;
   }

   Sprite&
   GetSprite();

   void
   SetPlayersPosition(const glm::vec2& position);

   std::shared_ptr< GameObject >
   GetGameObjectOnLocation(const glm::vec2& screenPosition);

   void
   MoveCamera(const glm::vec2& moveBy);

   glm::ivec2
   CheckMoveCamera(const glm::vec2& moveBy) const;

   glm::ivec2
   GetTilePosition(const glm::vec2& position) const;

 private:
   Logger m_logger = Logger("Level");

   Application* m_contextPointer = nullptr;
   Sprite m_background;
   Texture m_collision;

   Shaders m_shaders{};
   glm::vec2 m_cameraPosition;
   std::shared_ptr< Player > m_player = nullptr;

   bool m_locked = false;

   glm::ivec2 m_levelSize;
   std::vector< std::shared_ptr< GameObject > > m_objects;
   PathFinder m_pathinder{};

   // Tile handling stuff (deprecated)
   std::unordered_map< std::string, Texture > m_textures;
   glm::ivec2 m_cameraTilePos;
   glm::ivec2 m_tileSize;
   glm::ivec2 m_numTuilesOnScreen;
   glm::ivec2 m_tilesToDraw;
};

} // namespace dgame