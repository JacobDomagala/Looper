#pragma once

#include "path_finder.hpp"
#include "player.hpp"

#include <glm/glm.hpp>
#include <unordered_map>

namespace looper {

class Application;
class GameObject;
class Game;

class Level
{
 public:
   std::shared_ptr< GameObject >
   AddGameObject(ObjectType objectType, const glm::vec2& position);

   [[nodiscard]] std::vector< Tile >
   GetTilesFromBoundingBox(const std::array< glm::vec2, 4 >& box) const;

   [[nodiscard]] std::vector< Tile >
   GetTilesFromRectangle(const std::array< glm::vec2, 4 >& rect) const;

   [[nodiscard]] Tile
   GetTileFromPosition(const glm::vec2& local) const;

   void
   MoveObjs(const glm::vec2& moveBy);

   /**
    * \brief Called whenever an ombject with collision is moved. Generates the vector of
    * occupied tiles/nodes, based on \c box
    *
    * \param[in] box Object's bounding box, needed to calculate collision
    * \param[in] currentTiles Tiles occupied by the object, up to this point
    * \param[in] objectID ID of the object that was moved
    * \param[in] hasCollision Whether moved object has collision
    *
    * \return Vector of tiles/nodes occupied by given object
    */
   std::vector< Tile >
   GameObjectMoved(const std::array< glm::vec2, 4 >& box, const std::vector< Tile >& currentTiles,
                   Object::ID objectID, bool hasCollision);

   void
   FreeNodes(Object::ID object, const std::vector< Tile >& nodes, bool hasCollision);

   void
   OccupyNodes(Object::ID object, const std::vector< Tile >& nodes, bool hasCollision);

   void
   Create(Application* context, const std::string& name, const glm::ivec2& size);

   // pathToLevel - global path to level file (.dgl)
   void
   Load(Application* context, const std::string& pathToLevel);

   // pathToLevel - global path to level file (.dgl)
   void
   Save(const std::string& pathToLevel);

   void
   Quit();

   void
   LoadPremade(const std::string& fileName, const glm::ivec2& size);

   void
   LoadShaders(const std::string& shaderName);

   void
   DeleteObject(Object::ID deletedObject);

   Object&
   GetObjectRef(Object::ID objectID);

   std::vector< std::shared_ptr< GameObject > >
   GetObjects(const std::vector< Object::ID >& objectIDs) const;

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
   [[nodiscard]] bool
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
   [[nodiscard]] std::vector< Tile >
   GetTilesAlongTheLine(const glm::vec2& fromPos, const glm::vec2& toPos) const;

   void
   GenerateTextureForCollision();

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

   [[nodiscard]] bool
   IsCameraLocked() const
   {
      return m_locked;
   }

   [[nodiscard]] glm::vec2
   GetLevelPosition() const
   {
      return m_background.GetPosition();
   }

   [[nodiscard]] glm::ivec2
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

   [[nodiscard]] const std::vector< std::shared_ptr< GameObject > >&
   GetObjects() const;

   [[nodiscard]] std::string
   GetShader() const
   {
      return m_shaderName;
   }

   void
   RenderPathfinder(bool render);

   void
   UpdateCollisionTexture();

   renderer::Sprite&
   GetSprite();

   void
   SetPlayersPosition(const glm::vec2& position);

   std::shared_ptr< GameObject >
   GetGameObjectOnLocation(const glm::vec2& screenPosition);

   std::shared_ptr< GameObject >
   GetGameObjectOnLocationAndLayer(const glm::vec2& screenPosition, int32_t renderLayer);

   [[nodiscard]] uint32_t
   GetTileSize() const
   {
      return m_tileWidth;
   }

 private:
   Application* m_contextPointer = nullptr;
   renderer::Sprite m_background = {};
   PathFinder m_pathFinder = {};

   // Base texture and collision texture
   renderer::TextureID baseTexture_ = {};
   renderer::TextureID collisionTexture_ = {};
   FileManager::ImageData collisionTextureData_ = {};

   std::string m_shaderName = "DefaultShader";
   std::shared_ptr< Player > m_player = nullptr;

   bool m_locked = false;

   std::string m_name = "DummyName";
   glm::ivec2 m_levelSize = {0, 0};
   uint32_t m_tileWidth = 128;
   std::vector< std::shared_ptr< GameObject > > m_objects = {};
};

} // namespace looper