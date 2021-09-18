#include "PathFinder.hpp"

#include <algorithm>
#include <list>

namespace dgame {

PathFinder::PathFinder(const glm::ivec2& levelSize, const uint32_t tileSize,
                       std::vector< Node >&& nodes)
   : m_initialized(false), m_nodes(nodes), m_levelSize(levelSize), m_tileSize(tileSize)
{
}

void
PathFinder::InitializeEmpty(const glm::ivec2& levelSize, const uint32_t tileSize)
{
   m_levelSize = levelSize;
   m_tileSize = tileSize;
}

void
PathFinder::Initialize(const glm::ivec2& levelSize, const uint32_t tileSize)
{
   InitializeEmpty(levelSize, tileSize);

   const auto grad = static_cast< int32_t >(tileSize);

   const auto w = levelSize.x / grad;
   const auto h = levelSize.y / grad;
   const auto offset =
      glm::vec2(static_cast< float >(grad) / 2.0f, static_cast< float >(grad) / 2.0f);

   // height
   for (int y = 0; y < h; ++y)
   {
      // width
      for (int x = 0; x < w; ++x)
      {
         bool obstacle = false;

         std::vector< Node::NodeID > connectedTo{};

         if (y > 0)
         {
            connectedTo.push_back(x + (y - 1) * w);
         }

         if (y < h - 1)
         {
            connectedTo.push_back(x + (y + 1) * w);
         }

         if (x > 0)
         {
            connectedTo.push_back((x - 1) + y * w);
         }

         if (x < w - 1)
         {
            connectedTo.push_back((x + 1) + y * w);
         }

         Node node(glm::ivec2{x, y}, glm::vec2(x * grad, y * grad) + offset, x + y * w,
                   connectedTo);
         node.m_occupied = obstacle;


         AddNode(std::move(node));
      }
   }

   m_initialized = true;
}

void
PathFinder::AddNode(Node&& newNode)
{
   m_nodes.push_back(std::move(newNode));
}

void
PathFinder::DeleteNode(Node::NodeID nodeToDelete)
{
   m_nodes.erase(std::find_if(m_nodes.begin(), m_nodes.end(), [nodeToDelete](const auto& node) {
      return nodeToDelete == node.m_ID;
   }));

   if (m_nodes.empty())
   {
      m_initialized = false;
   }
}

const std::vector< Node >&
PathFinder::GetAllNodes() const
{
   return m_nodes;
}

std::vector< Node >&
PathFinder::GetAllNodes()
{
   return m_nodes;
}

Node::NodeID
PathFinder::GetNodeIDFromPosition(const glm::vec2& position) const
{
   if (position.x < 0 or position.x >= static_cast< float >(m_levelSize.x) or position.y < 0
       or position.y >= static_cast< float >(m_levelSize.y))
   {
      return -1;
   }

   const auto w = static_cast< int32_t >(glm::floor(position.x / static_cast< float >(m_tileSize)));
   const auto h = static_cast< int32_t >(glm::floor(position.y / static_cast< float >(m_tileSize)));

   const auto nodeFound = std::find_if(m_nodes.begin(), m_nodes.end(), [w, h](const auto& node) {
      return node.m_xPos == w and node.m_yPos == h;
   });

   assert(nodeFound != m_nodes.end());

   return nodeFound->m_ID;
}

Node&
PathFinder::GetNodeFromPosition(const glm::vec2& position)
{
   return GetNodeFromID(GetNodeIDFromPosition(position));
}

Node&
PathFinder::GetNodeFromID(Node::NodeID ID)
{
   const auto nodeFound = std::find_if(m_nodes.begin(), m_nodes.end(),
                                       [ID](const auto& node) { return node.m_ID == ID; });

   assert(nodeFound != m_nodes.end());

   return *nodeFound;
}

Node::NodeID
PathFinder::GetNodeIDFromTile(const glm::ivec2& tile) const
{
   const auto nodeFound = std::find_if(m_nodes.begin(), m_nodes.end(), [tile](const auto& node) {
      return node.m_xPos == tile.x and node.m_yPos == tile.y;
   });

   assert(nodeFound != m_nodes.end());

   return nodeFound->m_ID;
}

std::vector< Node::NodeID >
PathFinder::GetPath(const glm::vec2& source, const glm::vec2& destination)
{
   auto& nodeStart = GetNodeFromPosition(source);
   auto& nodeEnd = GetNodeFromPosition(destination);

   // Reset Navigation Graph - default all node states
   std::for_each(m_nodes.begin(), m_nodes.end(), [](auto& node) {
      node.m_parentNode = -1;
      node.m_visited = false;
      node.m_localCost = std::numeric_limits< int32_t >::max();
      node.m_globalCost = std::numeric_limits< int32_t >::max();
   });

   // Setup starting conditions
   Node* nodeCurrent = &nodeStart;
   nodeStart.m_localCost = 0;
   nodeStart.m_globalCost =
      static_cast< int32_t >(glm::distance(nodeStart.m_position, nodeEnd.m_position));

   // Add start node to not tested list - this will ensure it gets tested.
   // As the algorithm progresses, newly discovered nodes get added to this
   // list, and will themselves be tested later
   std::list< Node* > listNotTestedNodes;
   listNotTestedNodes.push_back(&nodeStart);

   // if the not tested list contains nodes, there may be better paths
   // which have not yet been explored. However, we will also stop
   // searching when we reach the target - there may well be better
   // paths but this one will do - it wont be the longest.
   while (!listNotTestedNodes.empty()
          && *nodeCurrent != nodeEnd) // Find absolutely shortest path // && nodeCurrent != nodeEnd)
   {
      // Sort Untested nodes by global goal, so lowest is first
      listNotTestedNodes.sort(
         [](const Node* lhs, const Node* rhs) { return lhs->m_globalCost < rhs->m_globalCost; });

      // Front of listNotTestedNodes is potentially the lowest distance node. Our
      // list may also contain nodes that have been visited, so ditch these...
      while (!listNotTestedNodes.empty() && listNotTestedNodes.front()->m_visited)
         listNotTestedNodes.pop_front();

      // ...or abort because there are no valid nodes left to test
      if (listNotTestedNodes.empty())
         break;

      nodeCurrent = listNotTestedNodes.front();
      nodeCurrent->m_visited = true; // We only explore a node once


      // Check each of this node's neighbours...
      for (auto nodeNeighbourID : nodeCurrent->m_connectedNodes)
      {
         auto& nodeNeighbour = GetNodeFromID(nodeNeighbourID);

         // ... and only if the neighbour is not visited and is
         // not an obstacle, add it to NotTested List
         if (!nodeNeighbour.m_visited && !nodeNeighbour.m_occupied)
            listNotTestedNodes.push_back(&nodeNeighbour);

         // Calculate the neighbours potential lowest parent distance
         float fPossiblyLowerGoal =
            static_cast< float >(nodeCurrent->m_localCost)
            + glm::distance(nodeCurrent->m_position, nodeNeighbour.m_position);

         // If choosing to path through this node is a lower distance than what
         // the neighbour currently has set, update the neighbour to use this node
         // as the path source, and set its distance scores as necessary
         if (fPossiblyLowerGoal < static_cast< float >(nodeNeighbour.m_localCost))
         {
            nodeNeighbour.m_parentNode = nodeCurrent->m_ID;
            nodeNeighbour.m_localCost = static_cast< int32_t >(fPossiblyLowerGoal);

            // The best path length to the neighbour being tested has changed, so
            // update the neighbour's score. The heuristic is used to globally bias
            // the path algorithm, so it knows if its getting better or worse. At some
            // point the algo will realise this path is worse and abandon it, and then go
            // and search along the next best path.
            nodeNeighbour.m_globalCost = nodeNeighbour.m_localCost
                                         + static_cast< int32_t >(glm::distance(
                                            nodeNeighbour.m_position, nodeEnd.m_position));
         }
      }
   }

   std::vector< Node::NodeID > nodePath;

   // Assume we found the path
   auto* currentNode = &nodeEnd;
   while (*currentNode != nodeStart)
   {
      nodePath.push_back(currentNode->m_ID);
      if (currentNode->m_parentNode != -1)
      {
         currentNode = &GetNodeFromID(currentNode->m_parentNode);
      }
      else
      {
         break;
      }
   }

   return nodePath;
}

void
PathFinder::SetNodeOccupied(const Tile_t& nodeCoords, Object::ID objectID)
{
   if (nodeCoords != Tile_t{-1, -1})
   {
      auto node = std::find_if(m_nodes.begin(), m_nodes.end(), [nodeCoords](const auto& node) {
         return (node.m_xPos == nodeCoords.first) and (node.m_yPos == nodeCoords.second);
      });

      assert(node != m_nodes.end());

      node->m_occupied = true;
      node->m_objectsOccupyingThisNode.push_back(objectID);
   }
}

void
PathFinder::SetNodeFreed(const Tile_t& nodeCoords, Object::ID objectID)
{
   if (nodeCoords != Tile_t{-1, -1})
   {
      auto node = std::find_if(m_nodes.begin(), m_nodes.end(), [nodeCoords](const auto& node) {
         return (node.m_xPos == nodeCoords.first) and (node.m_yPos == nodeCoords.second);
      });

      assert(node != m_nodes.end());

      node->m_objectsOccupyingThisNode.erase(std::find(node->m_objectsOccupyingThisNode.begin(),
                                                       node->m_objectsOccupyingThisNode.end(),
                                                       objectID));

      if (node->m_objectsOccupyingThisNode.empty())
      {
         node->m_occupied = false;
      }
   }
}

void
PathFinder::SetInitialized()
{
   m_initialized = true;
}

bool
PathFinder::IsInitialized() const
{
   return m_initialized;
}

} // namespace dgame