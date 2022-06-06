#pragma once

#include "Common.hpp"
#include "Object.hpp"

#include <glm/glm.hpp>
#include <vector>

namespace dgame {

// Should be Tile probably
struct Node : public Object
{
   using NodeID = int32_t;

   Node() : Object(Object::TYPE::PATHFINDER_NODE)
   {
   }

   Node(const glm::ivec2& coords, const glm::vec2& posOnMap, NodeID nodeID,
        std::vector< NodeID > connectedTo = {}, bool occupied = false,
        std::vector< Object::ID > objectOccupying = {})
      : Object(Object::TYPE::PATHFINDER_NODE),
        m_xPos(coords.x),
        m_yPos(coords.y),
        m_position(posOnMap),
        m_occupied(occupied),
        m_ID(nodeID),
        m_connectedNodes(std::move(connectedTo)),
        m_objectsOccupyingThisNode(std::move(objectOccupying))
   {
   }

   // X.Y coords
   int32_t m_xPos = {};
   int32_t m_yPos = {};

   glm::vec2 m_position = {};

   bool m_occupied = false;

   NodeID m_ID = -1;

   // Node which updated this node
   NodeID m_parentNode = -1;

   std::vector< NodeID > m_connectedNodes = {};
   std::vector< Object::ID > m_objectsOccupyingThisNode = {};

   bool m_visited = false;
   int32_t m_localCost = std::numeric_limits< int32_t >::max();
   int32_t m_globalCost = std::numeric_limits< int32_t >::max();
};

inline bool
operator==(const Node& left, const Node& right)
{
   return left.m_ID == right.m_ID;
}

class PathFinder
{
 public:
   PathFinder() = default;
   PathFinder(const glm::ivec2& levelSize, uint32_t tileSize, std::vector< Node >&& nodes);

   /**
    * \brief Initialize Pathfinder. This will create nodes for entire Level.
    *
    * \param[in] levelSize Size of the current Level
    * \param[in] tileSize Size of tile
    */
   void
   Initialize(const glm::ivec2& levelSize, uint32_t tileSize);

   /**
    * \brief Initialize Pathfinder. This will not create nodes.
    *
    * \param[in] levelSize Size of the current Level
    * \param[in] tileSize Size of tile
    */
   void
   InitializeEmpty(const glm::ivec2& levelSize, uint32_t tileSize);

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
   DeleteNode(Node::NodeID nodeToDelete);

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
   [[nodiscard]] Node::NodeID
   GetNodeIDFromPosition(const glm::vec2& position) const;

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
   [[nodiscard]] Node::NodeID
   GetNodeIDFromTile(const glm::ivec2& tile) const;

   /**
    * \brief Get Node from NodeID
    *
    * \param[in] ID NodeID
    *
    * \return Node
    */
   Node&
   GetNodeFromID(Node::NodeID ID);

   /**
    * \brief Get path from \c source to \c destination. Uses A* algorithm.
    *
    * \param[in] source Starting point on the map
    * \param[in] destination Destination on the map
    *
    * \return Nodes along the way
    */
   std::vector< Node::NodeID >
   GetPath(const glm::vec2& source, const glm::vec2& destination);

   /**
    * \brief Set node (on the given tile) occupied
    *
    * \param[in] nodeCoords Tile on the map
    * \param[in] objectID Object that occupies this node/tile
    */
   void
   SetNodeOccupied(const Tile_t& nodeCoords, Object::ID objectID);

   /**
    * \brief Set node (on the given tile) freed
    *
    * \param[in] nodeCoords Tile on the map
    * \param[in] objectID Object that no longer occupies this node/tile
    */
   void
   SetNodeFreed(const Tile_t& nodeCoords, Object::ID objectID);

 private:
   bool m_initialized = false;
   std::vector< Node > m_nodes = {};
   glm::ivec2 m_levelSize = {};
   uint32_t m_tileSize = {};
};

} // namespace dgame