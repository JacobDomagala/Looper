#pragma once

#include "Utils.hpp"

#include <sstream>
#include <iomanip>

std::string CustomFloatToStr(float value, int precision)
{
      std::stringstream stream;
      stream << std::fixed << std::setprecision(precision) << value;

      return stream.str();
}