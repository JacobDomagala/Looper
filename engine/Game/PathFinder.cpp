#include <PathFinder.hpp>
#include <algorithm>

namespace dgame {

PathFinder::PathFinder()
{
}

PathFinder::PathFinder(std::vector< Node >&& nodes) : m_nodes(nodes)
{
   m_initialized = true;
}

void
PathFinder::AddNode(Node newNode)
{
   m_nodes.push_back(newNode);
   m_initialized = true;
}

void
PathFinder::DeleteNode(Node deletedNode)
{
   m_nodes.erase(std::find(m_nodes.begin(), m_nodes.end(), deletedNode));

   if (m_nodes.empty())
   {
      m_initialized = false;
   }
}

Node::NodeID
PathFinder::FindNodeIdx(const glm::ivec2& position) const
{
   auto nodeFound = std::find_if(m_nodes.begin(), m_nodes.end(), [position](const auto& node) { return node.m_position == position; });

   return (nodeFound != m_nodes.end()) ? nodeFound->m_ID : 0;
}

glm::ivec2
PathFinder::GetNearestPosition(/*const glm::ivec2& objectPos*/ Node::NodeID currIdx, const glm::ivec2& targetPos) const
{
   glm::ivec2 returnPos{};

   auto nodeFound = std::find_if(m_nodes.begin(), m_nodes.end(), [currIdx](const auto& node) { return node.m_ID == currIdx; });

   if (nodeFound != m_nodes.end())
   {
      const auto& nearestNode = *nodeFound;
      auto length = glm::length(static_cast< glm::vec2 >(targetPos - nearestNode.m_position));

      for (const auto& nodeIdx : nearestNode.m_connectedNodes)
      {
         auto currentNodeIt = std::find_if(m_nodes.begin(), m_nodes.end(), [nodeIdx](const auto& node) { return node.m_ID == nodeIdx; });

         if (currentNodeIt != m_nodes.end())
         {
            const auto& currentNode = *currentNodeIt;

            auto currentLength = glm::length(static_cast< glm::vec2 >(targetPos - currentNode.m_position));
            if (currentLength < length)
            {
               length = currentLength;
               returnPos = currentNode.m_position;
            }
         }
      }
   }

   return returnPos;
}

Node::NodeID
PathFinder::GetNearestNode(const glm::ivec2& position) const
{
   Node::NodeID nearestNode(-1);
   auto currentMinLen(FLT_MAX);

   for (const auto& node : m_nodes)
   {
      auto currentLength = glm::length(static_cast< glm::vec2 >(position - node.m_position));

      if (currentLength < currentMinLen)
      {
         nearestNode = node.m_ID;
         currentMinLen = currentLength;
      }
   }

   return nearestNode;
}

std::vector< Node >
PathFinder::GetAllNodes() const
{
   return m_nodes;
}

bool
PathFinder::IsInitialized() const
{
   return m_initialized;
}

} // namespace dgame