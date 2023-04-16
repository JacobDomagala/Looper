#pragma once

#include "types.hpp"
#include "utils/assert.hpp"
#include "vertex.hpp"
#include "buffer.hpp"

#include <array>
#include <fmt/format.h>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

struct GLFWwindow;

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

static constexpr uint32_t MAX_NUM_TEXTURES = 256;
static constexpr uint32_t INDICES_PER_SPRITE = 6;
static constexpr uint32_t VERTICES_PER_SPRITE = 4;
static constexpr uint32_t INDICES_PER_LINE = 2;
static constexpr uint32_t VERTICES_PER_LINE = 2;
inline constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 3;
static constexpr bool ENABLE_VALIDATION = true;
static constexpr std::array< const char*, 1 > VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};
static constexpr std::array< const char*, 1 > DEVICE_EXTENSIONS = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct RenderData
{  
   // Vertex
   std::vector< Vertex > vertices = {};
   VkBuffer vertexBuffer = VK_NULL_HANDLE;
   VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;

   // Index
   std::vector< IndexType > indices = {};
   VkBuffer indexBuffer = VK_NULL_HANDLE;
   VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
   
   // UBO (UniformBufferObject)
   std::vector< VkBuffer > uniformBuffers = {MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE};
   std::vector< VkDeviceMemory > uniformBuffersMemory = {MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE};

   // SSBO (PerInstanceBuffer) 
   std::vector< PerInstanceBuffer > perInstance = {MAX_FRAMES_IN_FLIGHT, PerInstanceBuffer{}};
   std::vector< VkBuffer > ssbo = {MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE};
   std::vector< VkDeviceMemory > ssboMemory = {MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE};
   
   VkSurfaceKHR surface = {};
   GLFWwindow* windowHandle = nullptr;

   // Swapchain
   VkSwapchainKHR swapChain = {};
   VkExtent2D swapChainExtent = {};
   std::vector< VkImage > swapChainImages = {};
   std::vector< VkImageView > swapChainImageViews = {};
   std::vector< VkFramebuffer > swapChainFramebuffers = {};
   VkDescriptorSetLayout descriptorSetLayout = {};
   VkDescriptorPool descriptorPool = {};
   std::vector< VkDescriptorSet > descriptorSets = {};
   VkFormat swapChainImageFormat = {};

   VkRenderPass renderPass = VK_NULL_HANDLE;
   VkPipeline pipeline = VK_NULL_HANDLE;
   VkPipelineCache pipelineCache = VK_NULL_HANDLE;
   VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
   
   VkImage depthImage = VK_NULL_HANDLE;
   VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
   VkImageView depthImageView = VK_NULL_HANDLE;

   VkImage colorImage = VK_NULL_HANDLE;
   VkDeviceMemory colorImageMemory = VK_NULL_HANDLE;
   VkImageView colorImageView = VK_NULL_HANDLE;
   
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
   inline static VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

   inline static VkCommandPool commandPool = {};
   inline static std::vector< VkCommandBuffer > commandBuffers = {};

   // Line pipeline
   inline static VkPipeline linePipeline_ = {};
   inline static VkPipelineLayout linePipelineLayout_ = {};
   inline static VkDescriptorPool lineDescriptorPool = {};
   inline static VkDescriptorSetLayout lineDescriptorSetLayout_ = {};
   inline static std::vector< VkDescriptorSet > lineDescriptorSets_ = {};
   inline static VkBuffer lineVertexBuffer = {};
   inline static VkDeviceMemory lineVertexBufferMemory = {};
   inline static VkBuffer lineIndexBuffer = {};
   inline static VkDeviceMemory lineIndexBufferMemory = {};
   inline static std::vector< LineVertex > lineVertices_ = {};
   inline static std::vector< uint32_t > lineIndices_ = {};
   inline static std::vector< VkBuffer > lineUniformBuffers_ = {};
   inline static std::vector< VkDeviceMemory > lineUniformBuffersMemory_ = {};
   inline static uint32_t numGridLines = {};
   inline static uint32_t numLines = {};
   inline static uint32_t curDynLineIdx = {};

   inline static std::unordered_map< ApplicationType, RenderData > renderData_ = {};

   inline static uint32_t currentFrame_ = {};
};

struct EditorData
{
   // Pathfinder
   inline static std::vector< Vertex > pathfinderVertices_ = {};
   inline static std::vector< uint32_t > pathfinderIndices_ = {};
   inline static VkBuffer pathfinderVertexBuffer = {};
   inline static VkDeviceMemory pathfinderVertexBufferMemory = {};
   inline static VkBuffer pathfinderIndexBuffer = {};
   inline static VkDeviceMemory pathfinderIndexBufferMemory = {};
   inline static uint32_t numNodes_ = {};

   // Animation
   inline static std::vector< Vertex > animationVertices_ = {};
   inline static std::vector< uint32_t > animationIndices_ = {};
   inline static VkBuffer animationVertexBuffer = {};
   inline static VkDeviceMemory animationVertexBufferMemory = {};
   inline static VkBuffer animationIndexBuffer = {};
   inline static VkDeviceMemory animationIndexBufferMemory = {};
   inline static uint32_t numPoints_ = {};

   // Dynamic Lines
   inline static constexpr uint32_t MAX_NUM_LINES = 100000;
};


uint32_t
FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

} // namespace looper::renderer
