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
   PushWorkUnit(const Precondition& prec, const WorkUnit& work);

   void
   RunWorkUnits();

 private:
   Workers queue_;
};

} // namespace looper