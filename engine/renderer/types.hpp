#pragma once

#include <glm/glm.hpp>

#include <array>
#include <string>

namespace looper {

enum class ObjectType
{
   NONE = 0,
   ENEMY = 1,
   PLAYER = 2,
   OBJECT = 3,
   ANIMATION_POINT = 4,
   PATHFINDER_NODE = 8
};

}

namespace looper::renderer {

enum class ApplicationType
{
   EDITOR,
   GAME
};

enum class TextureType
{
   DIFFUSE_MAP = 0,
   SPECULAR_MAP = 1,
   NORMAL_MAP = 2
};

// DIFFUSE_MAP SPECULAR_MAP NORMAL_MAP
using TextureMaps = std::array< std::string, 3 >;

struct PerInstanceBuffer
{
   alignas(16) glm::mat4 model = {};
   glm::vec4 color = {};
   int32_t diffuse = {};
};

using IndexType = uint32_t;

} // namespace looper::renderer
