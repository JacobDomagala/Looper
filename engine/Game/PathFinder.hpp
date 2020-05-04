#pragma once

#include "Object.hpp"

#include <glm/glm.hpp>
#include <vector>

struct Node : public Object
{
   Node() = default;

   Node(glm::ivec2 pos, uint8_t ID, std::vector< uint8_t > connectedTo) : Object(Object::TYPE::PATHFINDER_NODE)
   {
      m_position = pos;
      m_ID = ID;
      m_connectedNodes = connectedTo;
   }

   glm::ivec2 m_position;
   uint8_t m_ID;
   std::vector< uint8_t > m_connectedNodes;
};

class PathFinder
{
   std::vector< std::shared_ptr< Node > > m_nodes;

 public:
   PathFinder();
   PathFinder(std::vector< std::shared_ptr< Node > >&& nodes);

   void
   AddNode(std::shared_ptr< Node > newNode);

   uint8_t
   FindNodeIdx(const glm::ivec2& position) const;

   glm::ivec2
   GetNearestPosition(uint8_t currIdx, const glm::ivec2& targetPos) const;

   uint8_t
   GetNearestNode(const glm::ivec2& position) const;

   std::vector< std::shared_ptr< Node > >
   GetAllNodes() const;
};
