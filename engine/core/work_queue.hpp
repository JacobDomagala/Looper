#pragma once

#include <functional>
#include <vector>

namespace looper {

class WorkQueue
{
 public:
   using WorkUnit = std::function< void() >;
   using Precondition = std::function< bool() >;
   using Workers = std::vector< std::pair< Precondition, WorkUnit > >;

   void
   PushWorkUnit(const Precondition& prec, const WorkUnit& work)
   {
      queue_.push_back({prec, work});
   }

   void
   RunWorkUnits()
   {
      Workers tmpQueue;

      for (auto& unit : queue_)
      {
         if (unit.first())
         {
            unit.second();
         }
         else
         {
            tmpQueue.push_back(unit);
         }
      }

      queue_.swap(tmpQueue);
   }

 private:
   Workers queue_;
};

} // namespace looper