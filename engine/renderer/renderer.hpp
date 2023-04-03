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

enum class MarkerType
{
   BEGIN,
   END
};

class VulkanRenderer
{
 public:
   static void
   Initialize(GLFWwindow* windowHandle);

   static void
   CreateRenderPipeline();

   static void
   Draw(Application* app);

   [[nodiscard]] static uint32_t
   MeshLoaded(const std::vector< Vertex >& vertices_in, const TextureMaps& textures_in,
              const glm::mat4& modelMat);

   static void
   SubmitMeshData(const uint32_t idx, const glm::mat4& modelMat);

   //static void
   //SetMeshesMarker(ApplicationType type, MarkerType markerType)
   //{
   //   switch (type)
   //   {
   //      case ApplicationType::EDITOR: {
   //         markerType == MarkerType::BEGIN ? numEditorObjs_.first = m_numMeshes
   //                                         : numEditorObjs_.second = m_numMeshes;
   //      }
   //      break;
   //      case ApplicationType::GAME: {
   //         markerType == MarkerType::BEGIN ? numGameObjs_.first = m_numMeshes
   //                                         : numGameObjs_.second = m_numMeshes;
   //      }
   //      break;
   //   }
   //}

   static void
   SetAppMarker(ApplicationType type)
   {
      boundApplication_ = type;
   }

   [[nodiscard]]
   static ApplicationType
   GetCurrentlyBoundType()
   {
      return boundApplication_;
   }

   //[[nodiscard]] static std::pair < uint32_t, uint32_t>
   //GetNumMeshes(ApplicationType type)
   //{
   //   switch (type)
   //   {
   //      case ApplicationType::EDITOR: {
   //         return numEditorObjs_;
   //      }
   //      break;
   //      case ApplicationType::GAME: {
   //         return numGameObjs_;
   //      }
   //      break;
   //   }
   //}

   static void
   SetupData();

   static void
   CreateCommandBuffers(Application* app, uint32_t imageIndex);

   inline static glm::mat4 view_mat = {};
   inline static glm::mat4 proj_mat = {};
   inline static glm::mat4 model_mat = {};

 private:
   static void
   CreateInstance();

   static void
   CreateDevice();

   static void
   CreateSwapchain(GLFWwindow* windowHandle);

   static void
   CreateImageViews();

   static void
   CreateDescriptorSetLayout();

   static void
   CreateRenderPass();

   static void
   CreateCommandPool();

   static void
   CreateFramebuffers();

   static void
   CreateSyncObjects();

   static void
   CreatePipeline();

   static void
   CreatePipelineCache();

   static void
   CreateVertexBuffer();

   static void
   CreateIndexBuffer();

   static void
   CreateUniformBuffers();

   static void
   UpdateUniformBuffer(uint32_t currentImage);

   static void
   CreateDescriptorPool();

   static void
   CreateDescriptorSets();


   static void
   CreateDepthResources();

   static void
   CreateColorResources();

   static VkFormat
   FindSupportedFormat(const std::vector< VkFormat >& candidates, VkImageTiling tiling,
                       VkFormatFeatureFlags features);

   static VkFormat
   FindDepthFormat();

   // static bool
   // HasStencilComponent(VkFormat format);

   //inline static uint32_t m_numMeshes = {};
   //inline static std::pair< uint32_t, uint32_t > numEditorObjs_ = {};
   //inline static std::pair< uint32_t, uint32_t > numGameObjs_ = {};

   inline static ApplicationType boundApplication_ = {};

 private:
   inline static bool isLoaded_ = false;
   inline static VkDebugUtilsMessengerCreateInfoEXT m_debugCreateInfo = {};
   inline static VkDebugUtilsMessengerEXT m_debugMessenger = {};
   inline static VkSurfaceKHR m_surface = {};

   inline static VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;

   inline static VkQueue m_presentQueue = {};

   inline static VkSwapchainKHR m_swapChain = {};
   inline static std::vector< VkImage > m_swapChainImages = {};
   inline static std::vector< VkImageView > m_swapChainImageViews = {};
   inline static std::vector< VkFramebuffer > m_swapChainFramebuffers = {};
   inline static VkFormat m_swapChainImageFormat = {};

   inline static VkRenderPass m_renderPass = {};

   inline static VkDescriptorSetLayout m_descriptorSetLayout = {};
   inline static VkDescriptorPool m_descriptorPool = {};


   inline static std::vector< VkCommandBuffer > m_commandBuffers = {};

   inline static std::vector< VkSemaphore > m_imageAvailableSemaphores = {};
   inline static std::vector< VkSemaphore > m_renderFinishedSemaphores = {};
   inline static std::vector< VkFence > m_inFlightFences = {};
   inline static std::vector< VkFence > m_imagesInFlight = {};

   //inline static std::vector< VkBuffer > m_uniformBuffers = {};
   //inline static std::vector< VkDeviceMemory > m_uniformBuffersMemory = {};

   //inline static std::vector< VkBuffer > m_ssbo = {};
   //inline static std::vector< VkDeviceMemory > m_ssboMemory = {};

   inline static VkImage m_depthImage = {};
   inline static VkDeviceMemory m_depthImageMemory = {};
   inline static VkImageView m_depthImageView = {};

   inline static VkImage m_colorImage = {};
   inline static VkDeviceMemory m_colorImageMemory = {};
   inline static VkImageView m_colorImageView = {};

   inline static std::vector< VkDrawIndexedIndirectCommand > m_renderCommands = {};
   inline static VkBuffer m_indirectDrawsBuffer = {};
   inline static VkDeviceMemory m_indirectDrawsBufferMemory = {};
   inline static uint32_t m_currentVertex = {};
   inline static uint32_t m_currentIndex = {};
};

} // namespace looper::renderer
