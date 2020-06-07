#pragma once

#include "Object.hpp"

#include <glm/glm.hpp>
#include <vector>

namespace dgame {

struct Node : public Object
{
   using NodeID = uint8_t;

   Node() = default;

   Node(glm::ivec2 pos, NodeID ID, std::vector< NodeID > connectedTo) : Object(Object::TYPE::PATHFINDER_NODE)
   {
      m_position = pos;
      m_ID = ID;
      m_connectedNodes = connectedTo;
   }

   glm::ivec2 m_position;
   NodeID m_ID;
   std::vector< NodeID > m_connectedNodes;
};

class PathFinder
{
   std::vector< std::shared_ptr< Node > > m_nodes;

 public:
   PathFinder();
   PathFinder(std::vector< std::shared_ptr< Node > >&& nodes);

   void
   AddNode(std::shared_ptr< Node > newNode);

   void
   DeleteNode(std::shared_ptr< Node > deletedNode);

   uint8_t
   FindNodeIdx(const glm::ivec2& position) const;

   glm::ivec2
   GetNearestPosition(uint8_t currIdx, const glm::ivec2& targetPos) const;

   uint8_t
   GetNearestNode(const glm::ivec2& position) const;

   std::vector< std::shared_ptr< Node > >
   GetAllNodes() const;
};

} // namespace dgame