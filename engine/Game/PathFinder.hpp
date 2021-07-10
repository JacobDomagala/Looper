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

   Node(glm::ivec2 coords, glm::ivec2 posOnMap, NodeID ID, std::vector< NodeID > connectedTo)
      : Object(Object::TYPE::PATHFINDER_NODE)
   {
      m_position = posOnMap;
      m_ID = ID;
      m_connectedNodes = connectedTo;

      m_xPos = coords.x;
      m_yPos = coords.y;
   }

   // X.Y coords
   int32_t m_xPos = {};
   int32_t m_yPos = {};

   // Map position
   bool m_occupied = false;
   glm::ivec2 m_position = {};

   NodeID m_ID = -1;
   std::vector< NodeID > m_connectedNodes = {};

   bool m_visited = false;
   int32_t m_localCost = 0;
};

class PathFinder
{
 public:
   PathFinder();
   PathFinder(std::vector< Node >&& nodes);

   void
   AddNode(Node newNode);

   void
   DeleteNode(Node deletedNode);

   Node::NodeID
   FindNodeIdx(const glm::ivec2& position) const;

   glm::ivec2
   GetNearestPosition(Node::NodeID currIdx, const glm::ivec2& targetPos) const;

   Node::NodeID
   GetNearestNode(const glm::ivec2& position) const;

   const std::vector< Node >&
   GetAllNodes() const;

   std::vector< Node >&
   GetAllNodes();

   void
   SetNodeOccupied(const std::pair<int32_t, int32_t>& nodeCoords);

   void
   SetNodeFreed(const std::pair< int32_t, int32_t >& nodeCoords);

   bool
   IsInitialized() const;

 private:
   bool m_initialized = false;
   std::vector< Node > m_nodes;
};

} // namespace dgame