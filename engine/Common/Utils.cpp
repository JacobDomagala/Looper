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
IsPositionClose(const glm::vec2& targetPos, const glm::vec2& startPos, const glm::vec2& currentPos, float mariginValue)
{
   bool xCheck = false;
   bool yCheck = false;

   const auto targetPosWithPositiveMarigin = targetPos + mariginValue;
   const auto targetPosWithNegativeMarigin = targetPos - mariginValue;

   // calculate the direction
   const auto direction = targetPos - startPos;
   if (direction.x >= 0)
   {
      xCheck = currentPos.x >= targetPosWithNegativeMarigin.x;
   }
   else
   {
      xCheck = currentPos.x <= targetPosWithPositiveMarigin.x;
   }

   if (direction.y >= 0)
   {
      yCheck = currentPos.y >= targetPosWithNegativeMarigin.y;
   }
   else
   {
      yCheck = currentPos.y <= targetPosWithPositiveMarigin.y;
   }

   return xCheck && yCheck;
}