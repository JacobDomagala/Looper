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
   EDITOR_OBJECT = 5,
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

enum class RotationType
{
   degrees,
   radians
};

enum class SpriteType
{
   regular,
   alwaysOnTop
};

enum class ApplicationType
{
   EDITOR,
   GAME
};

enum class TextureType
{
   DIFFUSE_MAP = 0,
   MASK_MAP = 1,
   SPECULAR_MAP = 2,
   NORMAL_MAP = 3
};

// DIFFUSE_MAP MASK_MAP SPECULAR_MAP NORMAL_MAP
using TextureMaps = std::array< std::string, 4 >;
using TextureID = int32_t;
using TextureIDs = std::array< TextureID, 4 >;

struct UniformBufferObject
{
   alignas(16) glm::mat4 proj = {};
   alignas(16) glm::mat4 view = {};
   glm::vec4 cameraPos = {};
   glm::vec4 lighting = {}; // enabled, ambient, intensity, coverage scale
   glm::vec4 viewport = {}; // viewport size, lighting capture UV offset
};

struct PerInstanceBuffer
{
   alignas(16) glm::mat4 model = {};
   glm::vec4 color = {};
   glm::vec4 texSamples = {};
   // RGB: emission. W: opacity for lit sprites, -(opacity + 1) for unlit sprites.
   // Keep the stride at 112 bytes so MAX_NUM_SPRITES fits a 128 MiB storage buffer.
   glm::vec4 emissionOpacity = {0.0f, 0.0f, 0.0f, -1.0f};
};

static_assert(sizeof(PerInstanceBuffer) == 112);

using IndexType = uint32_t;

} // namespace looper::renderer
