#pragma once

#include "shader.hpp"
#include "types.hpp"
#include "vertex.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>


struct GLFWwindow;

namespace looper {
class Application;
}

namespace looper::renderer {

class VulkanRenderer
{
 public:
   static void
   Initialize(GLFWwindow* windowHandle, ApplicationType type);

   static void
   CreateRenderPipeline();

   static void
   Render(Application* app);

   static void
   DrawLine(const glm::vec2& start, const glm::vec2& end);

   static void
   DrawDynamicLine(const glm::vec2& start, const glm::vec2& end);

   [[nodiscard]] static uint32_t
   MeshLoaded(const std::vector< Vertex >& vertices_in, const TextureMaps& textures_in,
              const glm::mat4& modelMat, const glm::vec4& color, ObjectType type);

   static void
   SubmitMeshData(const uint32_t idx, const TextureID textures_in, const glm::mat4& modelMat,
                  const glm::vec4& color);

   inline static void
   SetAppMarker(ApplicationType type)
   {
      boundApplication_ = type;
   }

   [[nodiscard]] inline static ApplicationType
   GetCurrentlyBoundType()
   {
      return boundApplication_;
   }

   static void
   CreateLinePipeline();

   static void
   SetupData(bool recreatePipeline = true);

   static void
   FreeData(renderer::ApplicationType type);

   static void
   UpdateBuffers();

   static void
   UpdatePerInstanceBuffer();

   static void
   SetupEditorData(ObjectType type);

   static void
   SetupLineData();

   static void
   UpdateLineData(uint32_t startingLine = 0);

   static void
   CreateCommandBuffers(Application* app, uint32_t imageIndex);

   static void
   UpdateDescriptors();

 private:
   static void
   DestroyPipeline();

   static void
   CreateInstance();

   static void
   CreateDevice();

   static void
   CreateSwapchain();

   static void
   CreateImageViews();

   static void
   CreateRenderPass();

   static void
   CreateCommandPool();

   static void
   CreateFramebuffers();

   static void
   CreateSyncObjects();

   static void
   CreatePipelineCache();

   static void
   CreateQuadVertexBuffer();

   static void
   CreateQuadIndexBuffer();

   static void
   CreateUniformBuffer();

   static void
   CreatePerInstanceBuffer();

   static void
   UpdateUniformBuffer(uint32_t currentImage);

   static void
   CreateDepthResources();

   static void
   CreateColorResources();

   static VkFormat
   FindSupportedFormat(const std::vector< VkFormat >& candidates, VkImageTiling tiling,
                       VkFormatFeatureFlags features);

   static VkFormat
   FindDepthFormat();

 private:
   inline static bool initialized_ = false;
   inline static bool isLoaded_ = false;
   inline static bool updateDescriptors_ = false;
   inline static VkDebugUtilsMessengerCreateInfoEXT m_debugCreateInfo = {};
   inline static VkDebugUtilsMessengerEXT m_debugMessenger = {};

   inline static VkQueue m_presentQueue = {};

   inline static VkRenderPass m_renderPass = {};

   inline static std::vector< VkSemaphore > m_imageAvailableSemaphores = {};
   inline static std::vector< VkSemaphore > m_renderFinishedSemaphores = {};
   inline static std::vector< VkFence > m_inFlightFences = {};
   inline static std::vector< VkFence > m_imagesInFlight = {};

   inline static ApplicationType boundApplication_ = {};

   // inline static std::vector< VkDrawIndexedIndirectCommand > m_renderCommands = {};
   // inline static VkBuffer m_indirectDrawsBuffer = {};
   // inline static VkDeviceMemory m_indirectDrawsBufferMemory = {};
   inline static uint32_t m_currentVertex = {};
   inline static uint32_t m_currentIndex = {};
};

} // namespace looper::renderer
