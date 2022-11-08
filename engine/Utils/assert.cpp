#include "assert.hpp"
#include "Logger/Logger.hpp"

namespace looper::utils {

void
Assert(bool assertion, std::string_view logMsg)
{
   if (!assertion)
   {
      //trace::Logger::Fatal(logMsg);
      std::terminate();
   }
}
} // namespace looper::utils