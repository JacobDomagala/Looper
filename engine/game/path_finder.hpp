#pragma once

#include "common.hpp"
#include "object.hpp"

#include <glm/glm.hpp>
#include <limits>
#include <unordered_set>
#include <vector>

#undef max

namespace looper {

class Level;

// Should be Tile probably
struct Node : public Object
{
   Node() : Object(ObjectType::PATHFINDER_NODE)
   {
   }

   Node(const glm::ivec2& coords, const glm::vec2& posOnMap, NodeID nodeID,
        std::vector< NodeID > connectedTo = {}, bool occupied = false,
        std::vector< Object::ID > objectOccupying = {})
      : Object(ObjectType::PATHFINDER_NODE),
        xPos_(coords.x),
        yPos_(coords.y),
        position_(posOnMap),
        occupied_(occupied),
        id_(nodeID),
        connectedNodes_(std::move(connectedTo)),
        objectsOccupyingThisNode_(std::move(objectOccupying))
   {
   }

   // X.Y coords
   int32_t xPos_ = {};
   int32_t yPos_ = {};

   glm::vec2 position_ = {};

   bool occupied_ = false;

   NodeID id_ = INVALID_NODE;

   // Node which updated this node
   NodeID parentNode_ = INVALID_NODE;

   std::vector< NodeID > connectedNodes_ = {};
   std::vector< Object::ID > objectsOccupyingThisNode_ = {};

   bool visited_ = false;
   int32_t localCost_ = std::numeric_limits< int32_t >::max();
   int32_t globalCost_ = std::numeric_limits< int32_t >::max();
};

inline bool
operator==(const Node& left, const Node& right)
{
   return left.id_ == right.id_;
}

class PathFinder
{
 public:
   PathFinder() = default;
   PathFinder(Level* level, std::vector< Node >&& nodes);

   /**
    * \brief Initialize Pathfinder. This will create nodes for entire Level.
    *
    * \param[in] level Level that created Pathfinder
    */
   void
   Initialize(Level* level);

   /**
    * \brief Initialize Pathfinder. This will not create nodes.
    *
    * \param[in] level Level that created Pathfinder
    */
   void
   InitializeEmpty(Level* level);

   /**
    * \brief Mark Pathfinder as initialized
    */
   void
   SetInitialized();

   /**
    * \brief Check whether the Pathfinder is initialized
    *
    * \return Whether it's initialized
    */
   [[nodiscard]] bool
   IsInitialized() const;

   /**
    * \brief Add node to the Pathfinder
    *
    * \param[in] newNode Node to add
    */
   void
   AddNode(Node&& newNode);

   /**
    * \brief Delete node from the Pathfinder
    *
    * \param[in] nodeToDelete Node to delete
    */
   void
   DeleteNode(NodeID nodeToDelete);

   /**
    * \brief Get nodes (const version)
    *
    * \return Nodes vector
    */
   [[nodiscard]] const std::vector< Node >&
   GetAllNodes() const;

   /**
    * \brief Get nodes (non-const version)
    *
    * \return Nodes vector
    */
   std::vector< Node >&
   GetAllNodes();

   /**
    * \brief Get NodeID from position
    *
    * \param[in] position Position on the map
    *
    * \return NodeID
    */
   [[nodiscard]] NodeID
   GetNodeIDFromPosition(const glm::vec2& position);

   /**
    * \brief Get Node from position
    *
    * \param[in] position Position on the map
    *
    * \return Node
    */
   Node&
   GetNodeFromPosition(const glm::vec2& position);

   /**
    * \brief Get NodeID from tile
    *
    * \param[in] tile Tile on the map
    *
    * \return NodeID
    */
   [[nodiscard]] NodeID
   GetNodeIDFromTile(const Tile& tile);

   /**
    * \brief Get Node from NodeID
    *
    * \param[in] ID NodeID
    *
    * \return Node
    */
   Node&
   GetNodeFromID(NodeID ID);

   /**
    * \brief Get path from \c source to \c destination. Uses A* algorithm.
    *
    * \param[in] source Starting point on the map
    * \param[in] destination Destination on the map
    *
    * \return Nodes along the way
    */
   std::vector< NodeID >
   GetPath(const glm::vec2& source, const glm::vec2& destination);

   /**
    * \brief Set node (on the given tile) occupied
    *
    * \param[in] nodeCoords Tile on the map
    * \param[in] objectID Object that occupies this node/tile
    */
   void
   SetNodeOccupied(const Tile& nodeCoords, Object::ID objectID);

   /**
    * \brief Set node (on the given tile) freed
    *
    * \param[in] nodeCoords Tile on the map
    * \param[in] objectID Object that no longer occupies this node/tile
    */
   void
   SetNodeFreed(const Tile& nodeCoords, Object::ID objectID);

   void
   ClearPerFrameData();

   const std::unordered_set< NodeID >&
   GetNodesModifiedLastFrame() const;

 private:
   bool initialized_ = false;
   std::vector< Node > nodes_ = {};
   std::unordered_set< NodeID > nodesModifiedLastFrame_ = {};
   Level* levelHandle_ = nullptr;
};

} // namespace looper