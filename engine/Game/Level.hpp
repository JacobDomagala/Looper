#pragma once

#include "PathFinder.hpp"
#include "Player.hpp"
#include "Shader.hpp"

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
   std::shared_ptr< GameObject >
   AddGameObject(GameObject::TYPE objectType);

   // Convert from OpenGL position to map position
   glm::vec2
   GetLocalVec(const glm::vec2& local) const;

   // Convert from map position to OpenGL
   glm::vec2
   GetGlobalVec(const glm::vec2& local) const;

   std::vector< std::pair< int32_t, int32_t > >
   GetTilesFromBoundingBox(const std::array<glm::vec2, 4>& box) const;

   std::pair<int32_t, int32_t>
   GetTileFromPosition(const glm::vec2& local) const;

   void
   MoveObjs(const glm::vec2& moveBy, bool isCameraMovement = true);

   std::vector< std::pair< int32_t, int32_t > >
   GameObjectMoved(const std::array< glm::vec2, 4 >& box,
                   const std::vector< std::pair< int32_t, int32_t > >& currentTiles);

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
   AddGameObject(Game& game, const glm::vec2& pos, const glm::ivec2& size,
                 const std::string& sprite);

   void
   DeleteObject(std::shared_ptr< Object > deletedObject);

   void
   DeleteObject(Object::ID deletedObject);

   Object&
   GetObjectRef(Object::ID object);

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

   // Renders only game objects
   void
   RenderGameObjects();

   bool
   IsInLevelBoundaries(const glm::vec2& position) const;

   bool
   CheckCollision(const glm::ivec2& localPos, const Player& player);

   bool
   CheckCollisionAlongTheLine(const glm::vec2& fromPos, const glm::vec2& toPos);

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
   GetLevelPosition() const
   {
      return m_background.GetPosition();
   }

   glm::ivec2
   GetSize() const
   {
      return m_levelSize;
   }

   void
   SetSize(const glm::ivec2& newSize);

   PathFinder&
   GetPathfinder()
   {
      return m_pathFinder;
   }

   std::shared_ptr< Player >
   GetPlayer()
   {
      return m_player;
   }

   std::vector< std::shared_ptr< GameObject > >
   GetObjects(bool includePlayer = false);

   std::string
   GetShader()
   {
      return m_shaderName;
   }

   Sprite&
   GetSprite();

   void
   SetPlayersPosition(const glm::vec2& position);

   std::shared_ptr< GameObject >
   GetGameObjectOnLocation(const glm::vec2& screenPosition);

   const Texture&
   GetCollision() const
   {
      return m_collision;
   }

   uint32_t
   GetTileSize() const
   {
      return m_tileWidth;
   }

 private:
   Logger m_logger = Logger("Level");

   Application* m_contextPointer = nullptr;
   Sprite m_background;
   Texture m_collision;

   std::string m_shaderName = "DefaultShader";
   std::shared_ptr< Player > m_player = nullptr;

   bool m_locked = false;

   glm::ivec2 m_levelSize = {0, 0};
   uint32_t m_tileWidth = 128;
   std::vector< std::shared_ptr< GameObject > > m_objects;
   PathFinder m_pathFinder;
};

} // namespace dgame