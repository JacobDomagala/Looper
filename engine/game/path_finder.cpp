#include "path_finder.hpp"
#include "level.hpp"
#include "utils/assert.hpp"

#include <algorithm>
#include <list>

namespace looper {

inline auto
GetNodeItFromTile(std::vector< Node >& nodes, const Tile& tile)
{
   auto nodeFound = stl::find_if(nodes, [tile](const auto& node) {
      return node.xPos_ == tile.first and node.yPos_ == tile.second;
   });

   utils::Assert(nodeFound != nodes.end(), "PathFinder::GetNodeIDFromPosition node not found!");

   return nodeFound;
}

auto
GetNodeItFromID(std::vector< Node >& nodes, NodeID id)
{
   auto nodeFound = stl::find_if(nodes, [id](const auto& node) { return node.id_ == id; });

   utils::Assert(nodeFound != nodes.end(), "PathFinder::GetNodeItFromID node not found!");

   return nodeFound;
}

PathFinder::PathFinder(Level* level, std::vector< Node >&& nodes)
   : nodes_(std::move(nodes)), levelHandle_(level)
{
}

void
PathFinder::InitializeEmpty(Level* level)
{
   levelHandle_ = level;
}

void
PathFinder::Initialize(Level* level)
{
   InitializeEmpty(level);

   const auto tileSize = levelHandle_->GetTileSize();
   const auto levelSize = levelHandle_->GetSize();

   const auto grad = static_cast< int32_t >(tileSize);

   const auto w = levelSize.x / grad;
   const auto h = levelSize.y / grad;
   const auto offset =
      glm::vec2(static_cast< float >(grad) / 2.0f, static_cast< float >(grad) / 2.0f);

   // TODO: parallelize!

   // height
   for (int y = 0; y < h; ++y)
   {
      // width
      for (int x = 0; x < w; ++x)
      {
         const bool obstacle = false;

         std::vector< NodeID > connectedTo{};

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
         node.occupied_ = obstacle;


         AddNode(std::move(node));
      }
   }

   initialized_ = true;
}

void
PathFinder::AddNode(Node&& newNode)
{
   nodes_.push_back(std::move(newNode));
}

void
PathFinder::DeleteNode(NodeID nodeToDelete)
{
   nodes_.erase(GetNodeItFromID(nodes_, nodeToDelete));

   if (nodes_.empty())
   {
      initialized_ = false;
   }
}

const std::vector< Node >&
PathFinder::GetAllNodes() const
{
   return nodes_;
}

std::vector< Node >&
PathFinder::GetAllNodes()
{
   return nodes_;
}

NodeID
PathFinder::GetNodeIDFromPosition(const glm::vec2& position)
{
   if (not levelHandle_->IsInLevelBoundaries(position))
   {
      return INVALID_NODE;
   }

   auto nodeFound = GetNodeItFromTile(nodes_, levelHandle_->GetTileFromPosition(position));

   return nodeFound->id_;
}

Node&
PathFinder::GetNodeFromPosition(const glm::vec2& position)
{
   return GetNodeFromID(GetNodeIDFromPosition(position));
}

Node&
PathFinder::GetNodeFromID(NodeID ID)
{
   return *GetNodeItFromID(nodes_, ID);
}

NodeID
PathFinder::GetNodeIDFromTile(const Tile& tile)
{
   return GetNodeItFromTile(nodes_, tile)->id_;
}

std::vector< NodeID >
PathFinder::GetPath(const glm::vec2& source, const glm::vec2& destination)
{
   auto& nodeStart = GetNodeFromPosition(source);
   auto& nodeEnd = GetNodeFromPosition(destination);

   // Reset Navigation Graph - default all node states
   stl::for_each(nodes_, [](auto& node) {
      node.parentNode_ = -1;
      node.visited_ = false;
      node.localCost_ = std::numeric_limits< int32_t >::max();
      node.globalCost_ = std::numeric_limits< int32_t >::max();
   });

   // Setup starting conditions
   Node* nodeCurrent = &nodeStart;
   nodeStart.localCost_ = 0;
   nodeStart.globalCost_ =
      static_cast< int32_t >(glm::distance(nodeStart.position_, nodeEnd.position_));

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
         [](const Node* lhs, const Node* rhs) { return lhs->globalCost_ < rhs->globalCost_; });

      // Front of listNotTestedNodes is potentially the lowest distance node. Our
      // list may also contain nodes that have been visited, so ditch these...
      while (!listNotTestedNodes.empty() && listNotTestedNodes.front()->visited_)
      {
         listNotTestedNodes.pop_front();
      }

      // ...or abort because there are no valid nodes left to test
      if (listNotTestedNodes.empty())
      {
         break;
      }

      nodeCurrent = listNotTestedNodes.front();
      nodeCurrent->visited_ = true; // We only explore a node once


      // Check each of this node's neighbours...
      for (auto nodeNeighbourID : nodeCurrent->connectedNodes_)
      {
         auto& nodeNeighbour = GetNodeFromID(nodeNeighbourID);

         // ... and only if the neighbour is not visited and is
         // not an obstacle, add it to NotTested List
         if (!nodeNeighbour.visited_ && !nodeNeighbour.occupied_)
         {
            listNotTestedNodes.push_back(&nodeNeighbour);
         }

         // Calculate the neighbours potential lowest parent distance
         const auto fPossiblyLowerGoal =
            static_cast< float >(nodeCurrent->localCost_)
            + glm::distance(nodeCurrent->position_, nodeNeighbour.position_);

         // If choosing to path through this node is a lower distance than what
         // the neighbour currently has set, update the neighbour to use this node
         // as the path source, and set its distance scores as necessary
         if (fPossiblyLowerGoal < static_cast< float >(nodeNeighbour.localCost_))
         {
            nodeNeighbour.parentNode_ = nodeCurrent->id_;
            nodeNeighbour.localCost_ = static_cast< int32_t >(fPossiblyLowerGoal);

            // The best path length to the neighbour being tested has changed, so
            // update the neighbour's score. The heuristic is used to globally bias
            // the path algorithm, so it knows if its getting better or worse. At some
            // point the algo will realise this path is worse and abandon it, and then go
            // and search along the next best path.
            nodeNeighbour.globalCost_ =
               nodeNeighbour.localCost_
               + static_cast< int32_t >(glm::distance(nodeNeighbour.position_, nodeEnd.position_));
         }
      }
   }

   std::vector< NodeID > nodePath;

   // Assume we found the path
   auto* currentNode = &nodeEnd;
   while (*currentNode != nodeStart)
   {
      nodePath.push_back(currentNode->id_);
      if (currentNode->parentNode_ != -1)
      {
         currentNode = &GetNodeFromID(currentNode->parentNode_);
      }
      else
      {
         break;
      }
   }

   return nodePath;
}

void
PathFinder::SetNodeOccupied(const Tile& nodeCoords, Object::ID objectID)
{
   if (nodeCoords != INVALID_TILE)
   {
      auto nodeFound = GetNodeItFromTile(nodes_, nodeCoords);
      nodeFound->occupied_ = true;
      nodeFound->objectsOccupyingThisNode_.push_back(objectID);

      nodesModifiedLastFrame_.insert(nodeFound->id_);
   }
}

void
PathFinder::SetNodeFreed(const Tile& nodeCoords, Object::ID objectID)
{
   if (nodeCoords != INVALID_TILE)
   {
      auto nodeFound = GetNodeItFromTile(nodes_, nodeCoords);

      auto objectFound = stl::find(nodeFound->objectsOccupyingThisNode_, objectID);

      utils::Assert(objectFound != nodeFound->objectsOccupyingThisNode_.end(),
                    fmt::format("PathFinder::SetNodeFreed object (ID:{}) not found!", objectID));

      nodeFound->objectsOccupyingThisNode_.erase(objectFound);

      if (nodeFound->objectsOccupyingThisNode_.empty())
      {
         nodeFound->occupied_ = false;
         nodesModifiedLastFrame_.insert(nodeFound->id_);
      }
   }
}

void
PathFinder::SetInitialized()
{
   initialized_ = true;
}

bool
PathFinder::IsInitialized() const
{
   return initialized_;
}

void
PathFinder::ClearPerFrameData()
{
   nodesModifiedLastFrame_.clear();
}

const std::unordered_set< NodeID >&
PathFinder::GetNodesModifiedLastFrame() const
{
   return nodesModifiedLastFrame_;
}

} // namespace looper