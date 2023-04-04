#pragma once

#include "types.hpp"
#include "utils/assert.hpp"
#include "vertex.hpp"

#include <array>
#include <fmt/format.h>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace looper::renderer {

constexpr inline void
vk_check_error(VkResult vkResult, std::string_view errorMessage)
{
   if (vkResult != VK_SUCCESS)
   {
      utils::Assert(false,
                    fmt::format("{} Return value {}", errorMessage, string_VkResult(vkResult)));
   }
}

static constexpr uint32_t indicesPerMesh = 6;
static constexpr bool ENABLE_VALIDATION = true;
static constexpr std::array< const char*, 1 > VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};
static constexpr std::array< const char*, 1 > DEVICE_EXTENSIONS = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct RenderData
{
   std::vector< Vertex > vertices = {};
   std::vector< IndexType > indices = {};
   std::vector< PerInstanceBuffer > perInstance = {};
   VkBuffer vertexBuffer = {};
   VkDeviceMemory vertexBufferMemory = {};
   VkBuffer indexBuffer = {};
   VkDeviceMemory indexBufferMemory = {};
   std::vector< VkBuffer > uniformBuffers = {};
   std::vector< VkDeviceMemory > uniformBuffersMemory = {};
   std::vector< VkBuffer > ssbo = {};
   std::vector< VkDeviceMemory > ssboMemory = {};
   VkSurfaceKHR surface = {};
   VkSwapchainKHR swapChain = {};
   std::vector< VkImage > swapChainImages = {};
   std::vector< VkImageView > swapChainImageViews = {};
   std::vector< VkFramebuffer > swapChainFramebuffers = {};
   VkFormat swapChainImageFormat = {};
   uint32_t numMeshes = {};
};

/*
 * This is storage for common Vulkan objects that are needed for numerous
 * function calls. All these fields are assigned by VulkanRenderer on Init
 * and should only be read from other modules.
 */
struct Data
{
   inline static VkInstance vk_instance = {};
   inline static VkDevice vk_device = {};
   inline static VkPhysicalDevice vk_physicalDevice = VK_NULL_HANDLE;
   inline static VkQueue vk_graphicsQueue = {};
   inline static VkCommandPool vk_commandPool = {};
   inline static VkPipelineCache m_pipelineCache = {};
   inline static VkRenderPass m_renderPass = {};
   inline static VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
   inline static VkExtent2D m_swapChainExtent = {};
   inline static uint32_t currentFrame_ = {};
   inline static std::vector< VkDescriptorSet > descriptorSets_ = {};
   inline static VkPipeline graphicsPipeline_ = {};
   inline static VkPipelineLayout pipelineLayout_ = {};

   inline static std::unordered_map< ApplicationType, RenderData > renderData_ = {};

   inline static const uint32_t MAX_FRAMES_IN_FLIGHT = 3;
};

uint32_t
FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

} // namespace looper::renderer
