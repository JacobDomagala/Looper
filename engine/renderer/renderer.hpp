#pragma once

#include "types.hpp"
#include "vulkan_common.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>


struct GLFWwindow;

namespace looper {
class Application;
}

namespace looper::renderer {

struct Vertex;

void
Initialize(GLFWwindow* windowHandle, ApplicationType type);

void
CreateRenderPipeline();

void
Render(Application* app);

void
DrawLine(const glm::vec2& start, const glm::vec2& end);

void
DrawDynamicLine(const glm::vec2& start, const glm::vec2& end);

void
MeshDeleted(const RenderInfo& renderInfo);

[[nodiscard]] RenderInfo
MeshLoaded(const std::vector< Vertex >& vertices_in, const TextureIDs& textures_in,
           const glm::mat4& modelMat, const glm::vec4& color);

void
SubmitMeshData(const uint32_t idx, const TextureIDs& ids, const glm::mat4& modelMat,
               const glm::vec4& color);

void
SetupVertexBuffer(const int32_t layer);

void
SetAppMarker(ApplicationType type);

RenderData&
GetRenderData();

[[nodiscard]] ApplicationType
GetCurrentlyBoundType();

void
CreateLinePipeline();

void
RecreateQuadPipeline();

void
FreeData(renderer::ApplicationType type, bool destroyPipeline);

void
CreateQuadBuffers();

void
UpdatePerInstanceBuffer();

void
SetupLineData();

void
UpdateLineData(uint32_t startingLine = 0);

void
CreateCommandBuffers(Application* app, uint32_t imageIndex);

void
UpdateDescriptors();

void
UpdateData();

void
WaitForFence(uint32_t frame);

} // namespace looper::renderer
