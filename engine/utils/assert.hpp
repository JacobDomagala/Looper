#pragma once

#include <string_view>

namespace looper::utils {

void
Assert(bool assertion, std::string_view logMsg);

} // namespace looper::utils