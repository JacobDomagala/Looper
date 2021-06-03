#pragma once

#include "Object.hpp"

#include <glm/glm.hpp>
#include <vector>

namespace dgame {

struct Node : public Object
{
   using NodeID = int32_t;

   Node() : Object(Object::TYPE::PATHFINDER_NODE)
   {
   }

   Node(glm::ivec2 posOnMap, NodeID ID, std::vector< NodeID > connectedTo) : Object(Object::TYPE::PATHFINDER_NODE)
   {
      m_position = posOnMap;
      m_ID = ID;
      m_connectedNodes = connectedTo;
   }

   // Map position
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

   bool
   IsInitialized() const;

 private:
   bool m_initialized = false;
   std::vector< Node > m_nodes;
};

} // namespace dgame