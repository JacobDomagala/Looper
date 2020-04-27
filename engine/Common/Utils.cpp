#pragma once

#include "Utils.hpp"

#include <iomanip>
#include <sstream>

std::string
CustomFloatToStr(float value, int precision)
{
   std::stringstream stream;
   stream << std::fixed << std::setprecision(precision) << value;

   return stream.str();
}


bool
IsPositionClose(const glm::vec2& targetPos, const glm::vec2& currentPos, float mariginValue)
{
   const auto targetPosWithPositiveMarigin = targetPos + mariginValue;
   const auto targetPosWithNegativeMarigin = targetPos - mariginValue;
   
   bool xRange = targetPosWithNegativeMarigin.x <= currentPos.x && targetPosWithPositiveMarigin.x >= currentPos.x;
   bool yRange = targetPosWithNegativeMarigin.y <= currentPos.y && targetPosWithPositiveMarigin.y >= currentPos.y;

   return xRange && yRange;
}