#pragma once

#include <glm/glm.hpp>

#include <array>
#include <ranges>
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

namespace stl = std::ranges;

using NodeID = int32_t;
constexpr NodeID INVALID_NODE = -1;

using byte_vec4 = glm::tvec4< uint8_t >;
using Tile = std::pair< int32_t, int32_t >;

constexpr Tile INVALID_TILE = Tile{-1, -1};

} // namespace looper

namespace looper::renderer {

enum class PrimitiveType
{
   TRIANGLE,
   LINE
};

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

struct UniformBufferObject
{
   alignas(16) glm::mat4 proj = {};
   alignas(16) glm::mat4 view = {};
   glm::vec4 cameraPos = {};
};

struct PerInstanceBuffer
{
   alignas(16) glm::mat4 model = {};
   glm::vec4 color = {};
   glm::vec4 texSamples = {};
};

using IndexType = uint32_t;
using TextureID = int32_t;

} // namespace looper::renderer
