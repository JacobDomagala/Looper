#pragma once

#include <renderer/types.hpp>
#include <renderer/vertex.hpp>

#include <vector>

namespace looper::editor {

struct RenderData
{
   static inline std::vector< renderer::PerInstanceBuffer > perInstance = {};
   static inline std::vector< renderer::Vertex > vertices = {};
   static inline std::vector< uint32_t > indices = {};
};

} // namespace looper::editor