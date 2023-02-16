#pragma once

#include <array>
#include <string>

namespace looper::render::vulkan {

enum class TextureType
{
   DIFFUSE_MAP = 0,
   SPECULAR_MAP = 1,
   NORMAL_MAP = 2
};

// DIFFUSE_MAP SPECULAR_MAP NORMAL_MAP
using TextureMaps = std::array< std::string, 3 >;

} // namespace looper::render::vulkan
