#include <PathFinder.hpp>
#include <algorithm>

namespace dgame {

PathFinder::PathFinder()
{
}

PathFinder::PathFinder(std::vector< std::shared_ptr< Node > >&& nodes) : m_nodes(nodes)
{
}

void
PathFinder::AddNode(std::shared_ptr< Node > newNode)
{
   m_nodes.push_back(newNode);
}

void
PathFinder::DeleteNode(std::shared_ptr< Node > deletedNode)
{
   m_nodes.erase(std::find(m_nodes.begin(), m_nodes.end(), deletedNode));
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

} // namespace dgame