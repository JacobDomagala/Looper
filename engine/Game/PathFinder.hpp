#pragma once

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

   Node(const glm::ivec2& coords, const glm::vec2& posOnMap, NodeID ID,
        const std::vector< NodeID >& connectedTo, bool occupied = false)
      : Object(Object::TYPE::PATHFINDER_NODE),
        m_xPos(coords.x),
        m_yPos(coords.y),
        m_position(posOnMap),
        m_occupied(occupied),
        m_ID(ID),
        m_connectedNodes(connectedTo)
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
   PathFinder();
   PathFinder(const glm::ivec2& levelSize, const uint32_t tileSize, std::vector< Node >&& nodes);

   void
   Initialize(const glm::ivec2& levelSize, const uint32_t tileSize);

   void
   InitializeEmpty(const glm::ivec2& levelSize, const uint32_t tileSize);


   void
   AddNode(Node newNode);

   void
   DeleteNode(Node deletedNode);

   Node::NodeID
   FindNodeIdx(const glm::vec2& position) const;

   glm::vec2
   GetNearestPosition(Node::NodeID currIdx, const glm::vec2& targetPos) const;

   Node::NodeID
   GetNearestNode(const glm::vec2& position) const;

   const std::vector< Node >&
   GetAllNodes() const;

   std::vector< Node >&
   GetAllNodes();

   Node::NodeID
   GetNodeIDFromPosition(const glm::vec2& position) const;

   Node::NodeID
   GetNodeIDFromTile(const glm::ivec2& tile) const;

   Node&
   GetNodeFromID(Node::NodeID ID);

   Node&
   GetNodeFromPosition(const glm::vec2& position);

   std::vector< Node::NodeID >
   GetPath(const glm::vec2& source, const glm::vec2& destination);

   void
   SetNodeOccupied(const std::pair< int32_t, int32_t >& nodeCoords);

   void
   SetNodeFreed(const std::pair< int32_t, int32_t >& nodeCoords);

   bool
   IsInitialized() const;

 private:
   bool m_initialized = false;
   std::vector< Node > m_nodes;
   glm::ivec2 m_levelSize = {};
   uint32_t m_tileSize = {};
};

} // namespace dgame