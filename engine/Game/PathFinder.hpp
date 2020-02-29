#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Node
{
   glm::ivec2 m_position;
   uint8_t m_ID;
   std::vector< uint8_t > m_connectedNodes;
};

class PathFinder
{
   std::vector< Node > m_nodes;

 public:
   PathFinder();
   PathFinder(std::vector< Node >&& nodes);

   uint8_t
   FindNodeIdx(const glm::ivec2& position) const;

   glm::ivec2
   GetNearestPosition(uint8_t currIdx, const glm::ivec2& targetPos) const;

   uint8_t
   GetNearestNode(const glm::ivec2& position) const;
};
