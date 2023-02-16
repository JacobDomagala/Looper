#include "assert.hpp"
#include "logger/logger.hpp"

namespace looper::utils {

void
Assert(bool assertion, std::string_view logMsg)
{
   if (!assertion)
   {
      Logger::Fatal("{}", logMsg);
      std::terminate();
   }
}
} // namespace looper::utils