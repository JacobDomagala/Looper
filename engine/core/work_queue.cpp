#include "work_queue.hpp"

#include <functional>
#include <vector>

namespace looper {

void
WorkQueue::PushWorkUnit(const WorkQueue::Precondition& prec, const WorkQueue::WorkUnit& work)
{
   queue_.push_back({prec, work});
}

void
WorkQueue::RunWorkUnits()
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

} // namespace looper