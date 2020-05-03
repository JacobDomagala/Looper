#include <PathFinder.hpp>
#include <algorithm>

PathFinder::PathFinder()
{
   /*std::vector< Node > nodes = {Node{{370, 1000}, 0, {1, 7}},   Node{{960, 1000}, 1, {0, 2}},    Node{{1600, 1000}, 2, {1, 3, 8, 13}},
                                Node{{1664, 1536}, 3, {2, 4}},  Node{{1600, 2200}, 4, {3, 5}},   Node{{960, 2200}, 5, {4, 6}},
                                Node{{370, 2200}, 6, {5, 7}},   Node{{370, 1600}, 7, {0, 6}},    Node{{2200, 1000}, 8, {2, 9}},
                                Node{{2800, 1000}, 9, {8, 10}}, Node{{2800, 1600}, 10, {9, 11}}, Node{{2700, 2100}, 11, {10, 12}},
                                Node{{2864, 2536}, 12, {11}},   Node{{1600, 600}, 13, {2, 14}},  Node{{1600, 135}, 14, {13, 15, 16}},
                                Node{{1300, 135}, 15, {14}},    Node{{1800, 135}, 16, {14}}};

   m_nodes = nodes;*/
}

PathFinder::PathFinder(std::vector< std::shared_ptr< Node > >&& nodes) : m_nodes(nodes)
{
}

void
PathFinder::AddNode(std::shared_ptr< Node > newNode)
{
   m_nodes.push_back(newNode);
}

uint8_t
PathFinder::FindNodeIdx(const glm::ivec2& position) const
{
   auto nodeFound = std::find_if(m_nodes.begin(), m_nodes.end(), [position](const auto& node) { return node->m_position == position; });

   return (nodeFound != m_nodes.end()) ? (*nodeFound)->m_ID : 0;
}

glm::ivec2
PathFinder::GetNearestPosition(/*const glm::ivec2& objectPos*/ uint8_t currIdx, const glm::ivec2& targetPos) const
{
   glm::ivec2 returnPos{};

   auto nodeFound = std::find_if(m_nodes.begin(), m_nodes.end(), [currIdx](const auto& node) { return node->m_ID == currIdx; });

   if (nodeFound != m_nodes.end())
   {
      auto nearestNode = *nodeFound;
      auto length = glm::length(static_cast< glm::vec2 >(targetPos - nearestNode->m_position));

      for (const auto& nodeIdx : nearestNode->m_connectedNodes)
      {
         auto currentNodeIt = std::find_if(m_nodes.begin(), m_nodes.end(), [nodeIdx](const auto& node) { return node->m_ID == nodeIdx; });

         if (currentNodeIt != m_nodes.end())
         {
            auto currentNode = *currentNodeIt;

            auto currentLength = glm::length(static_cast< glm::vec2 >(targetPos - currentNode->m_position));
            if (currentLength < length)
            {
               length = currentLength;
               returnPos = currentNode->m_position;
            }
         }
      }
   }

   return returnPos;
}

uint8_t
PathFinder::GetNearestNode(const glm::ivec2& position) const
{
   uint8_t nearestNode(0);
   auto tmpLength(FLT_MAX);

   for (const auto& node : m_nodes)
   {
      auto currentLength = glm::length(static_cast< glm::vec2 >(position - node->m_position));

      if (currentLength < tmpLength)
      {
         nearestNode = node->m_ID;
         tmpLength = currentLength;
      }
   }

   return nearestNode;
}

std::vector< std::shared_ptr< Node > >
PathFinder::GetAllNodes() const
{
   return m_nodes;
}