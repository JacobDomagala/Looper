#pragma once

#include "Common.h"

struct Node
{
    glm::ivec2          m_position;
    uint8_t             m_ID;
    std::vector< uint8_t > m_connectedNodes;
};

class PathFinder
{
    std::vector< Node > m_nodes;

    std::pair< const Node&, bool > FindNode( const glm::ivec2& position );

 public:
	PathFinder();
    PathFinder( std::vector< Node >&& nodes );

	uint8_t FindNodeIdx(const glm::ivec2& position) const;
    glm::ivec2 GetNearestPosition(/* const glm::ivec2& objectPos*/ uint8_t currIdx, const glm::ivec2& targetPos ) const;
};
