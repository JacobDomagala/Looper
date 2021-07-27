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
   static constexpr Tile_t invalidTile = Tile_t{-1, -1};

 public:
   std::shared_ptr< GameObject >
   AddGameObject(GameObject::TYPE objectType);

   // Convert from OpenGL position to map position
   glm::vec2
   GetLocalVec(const glm::vec2& local) const;

   // Convert from map position to OpenGL
   glm::vec2
   GetGlobalVec(const glm::vec2& local) const;

   std::vector< Tile_t >
   GetTilesFromBoundingBox(const std::array< glm::vec2, 4 >& box) const;

   Tile_t
   GetTileFromPosition(const glm::vec2& local) const;

   void
   MoveObjs(const glm::vec2& moveBy, bool isCameraMovement = true);

   std::vector< Tile_t >
   GameObjectMoved(const std::array< glm::vec2, 4 >& box,
                   const std::vector< Tile_t >& currentTiles);

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
   DeleteObject(Object::ID deletedObject);

   Object&
   GetObjectRef(Object::ID object);

   void
   Update(bool isReverse);

   void
   Render();

   /**
    * \brief Renders game objects
    */
   void
   RenderGameObjects();

   /**
    * \brief Checks whether \c position is inside level boundaries
    *
    * \param[in] position Position to check
    *
    * \return True if it's on the map, false otherwise
    */
   bool
   IsInLevelBoundaries(const glm::vec2& position) const;

   /**
    * \brief Get collided position along the line (from-to).
    *
    * \return Return the collided position or \c toPos if nothing was on the path
    */
   glm::vec2
   GetCollidedPosition(const glm::vec2& fromPos, const glm::vec2& toPos);

   /**
    * \brief Checks collision along the line (fromPos - toPos)
    *
    * \param[in] fromPos Starting position
    * \param[in] toPos Ending position
    *
    * \return True if not collided, False otherwise
    */
   bool
   CheckCollisionAlongTheLine(const glm::vec2& fromPos, const glm::vec2& toPos);

   /**
    * \brief Get tiles along the line (fromPos - toPos)
    *
    * \param[in] fromPos Starting position
    * \param[in] toPos Ending position
    *
    * \return Vector of tiles
    */
   std::vector< Tile_t >
   GetTilesAlongTheLine(const glm::vec2& fromPos, const glm::vec2& toPos) const;

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

   const std::vector< std::shared_ptr< GameObject > >&
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

   uint32_t
   GetTileSize() const
   {
      return m_tileWidth;
   }

 private:
   Logger m_logger = Logger("Level");

   Application* m_contextPointer = nullptr;
   Sprite m_background;

   std::string m_shaderName = "DefaultShader";
   std::shared_ptr< Player > m_player = nullptr;

   bool m_locked = false;

   glm::ivec2 m_levelSize = {0, 0};
   uint32_t m_tileWidth = 128;
   std::vector< std::shared_ptr< GameObject > > m_objects;
   PathFinder m_pathFinder;
};

} // namespace dgame