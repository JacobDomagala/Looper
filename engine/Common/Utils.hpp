#pragma once

#include <glm/glm.hpp>
#include <string>

namespace dgame {

std::string
CustomFloatToStr(float value, int precision = 2);

bool
IsPositionClose(const glm::vec2& targetPos, const glm::vec2& startPos, const glm::vec2& currentPos, float mariginValue = 0.1f);

} // namespace dgame