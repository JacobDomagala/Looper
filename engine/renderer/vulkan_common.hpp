#pragma once

#include "buffer.hpp"
#include "types.hpp"
#include "utils/assert.hpp"
#include "vertex.hpp"

#include <array>
#include <bitset>
#include <fmt/format.h>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

struct GLFWwindow;

namespace looper::renderer {

constexpr inline const char*
string_VkResult(VkResult input_value)
{
   switch (input_value)
   {
      case VK_ERROR_DEVICE_LOST:
         return "VK_ERROR_DEVICE_LOST";
      case VK_ERROR_EXTENSION_NOT_PRESENT:
         return "VK_ERROR_EXTENSION_NOT_PRESENT";
      case VK_ERROR_FEATURE_NOT_PRESENT:
         return "VK_ERROR_FEATURE_NOT_PRESENT";
      case VK_ERROR_FORMAT_NOT_SUPPORTED:
         return "VK_ERROR_FORMAT_NOT_SUPPORTED";
      case VK_ERROR_FRAGMENTATION_EXT:
         return "VK_ERROR_FRAGMENTATION_EXT";
      case VK_ERROR_FRAGMENTED_POOL:
         return "VK_ERROR_FRAGMENTED_POOL";
      case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
         return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
      case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
         return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
      case VK_ERROR_INCOMPATIBLE_DRIVER:
         return "VK_ERROR_INCOMPATIBLE_DRIVER";
      case VK_ERROR_INITIALIZATION_FAILED:
         return "VK_ERROR_INITIALIZATION_FAILED";
      case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
         return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT";
      case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
         return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
      case VK_ERROR_INVALID_EXTERNAL_HANDLE:
         return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
      case VK_ERROR_INVALID_SHADER_NV:
         return "VK_ERROR_INVALID_SHADER_NV";
      case VK_ERROR_LAYER_NOT_PRESENT:
         return "VK_ERROR_LAYER_NOT_PRESENT";
      case VK_ERROR_MEMORY_MAP_FAILED:
         return "VK_ERROR_MEMORY_MAP_FAILED";
      case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
         return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
      case VK_ERROR_NOT_PERMITTED_EXT:
         return "VK_ERROR_NOT_PERMITTED_EXT";
      case VK_ERROR_OUT_OF_DATE_KHR:
         return "VK_ERROR_OUT_OF_DATE_KHR";
      case VK_ERROR_OUT_OF_DEVICE_MEMORY:
         return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
      case VK_ERROR_OUT_OF_HOST_MEMORY:
         return "VK_ERROR_OUT_OF_HOST_MEMORY";
      case VK_ERROR_OUT_OF_POOL_MEMORY:
         return "VK_ERROR_OUT_OF_POOL_MEMORY";
      case VK_ERROR_SURFACE_LOST_KHR:
         return "VK_ERROR_SURFACE_LOST_KHR";
      case VK_ERROR_TOO_MANY_OBJECTS:
         return "VK_ERROR_TOO_MANY_OBJECTS";
      case VK_ERROR_VALIDATION_FAILED_EXT:
         return "VK_ERROR_VALIDATION_FAILED_EXT";
      case VK_EVENT_RESET:
         return "VK_EVENT_RESET";
      case VK_EVENT_SET:
         return "VK_EVENT_SET";
      case VK_INCOMPLETE:
         return "VK_INCOMPLETE";
      case VK_NOT_READY:
         return "VK_NOT_READY";
      case VK_SUBOPTIMAL_KHR:
         return "VK_SUBOPTIMAL_KHR";
      case VK_SUCCESS:
         return "VK_SUCCESS";
      case VK_TIMEOUT:
         return "VK_TIMEOUT";
      default:
         return "Unhandled VkResult";
   }
}

constexpr inline void
vk_check_error(VkResult vkResult, std::string_view errorMessage)
{
   if (vkResult != VK_SUCCESS)
   {
      utils::Assert(false,
                    fmt::format("{} Return value {}", errorMessage, string_VkResult(vkResult)));
   }
}

static constexpr uint32_t VERTICES_PER_SPRITE = 4;
static constexpr uint32_t INDICES_PER_SPRITE = 6;
static constexpr uint32_t MAX_SPRITES_PER_LAYER = 100000;
static constexpr size_t MAX_NUM_VERTICES_PER_LAYER = MAX_SPRITES_PER_LAYER * VERTICES_PER_SPRITE;
static constexpr size_t MAX_NUM_INDICES_PER_LAYER = MAX_SPRITES_PER_LAYER * INDICES_PER_SPRITE;
inline constexpr uint32_t NUM_LAYERS = 11;
static constexpr size_t MAX_NUM_SPRITES = MAX_SPRITES_PER_LAYER * NUM_LAYERS;
static constexpr uint32_t MAX_NUM_TEXTURES = 256;
static constexpr uint32_t INDICES_PER_LINE = 2;
static constexpr uint32_t VERTICES_PER_LINE = 2;
inline constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
inline constexpr uint32_t MAX_NUM_LINES = 100000;

static constexpr bool ENABLE_VALIDATION = true;
static constexpr std::array< const char*, 1 > VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};
static constexpr std::array< const char*, 1 > DEVICE_EXTENSIONS = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct Vertex;

struct RenderInfo
{
   // perInstance index
   int32_t idx;

   // Layer number
   int32_t layer;

   // index in given layer
   int32_t layerIdx;
};

struct PushConstBlock
{
   glm::vec2 scale = {};
   glm::vec2 translate = {};
};

struct RenderData
{
   // Store this in case we have window minimized (to prevent extent being 0x0)
   glm::ivec2 windowSize_ = {};
   bool windowFocus_ = true;

   std::array< std::vector< uint32_t >, NUM_LAYERS > deletedObjs_ = {};
   ////////////////////////////////////////////////////////////////////////
   // RENDER LAYERS (from near to far (values ranging from 0.0 to -0.9))
   ////////////////////////////////////////////////////////////////////////

   // Vertex
   std::array< std::vector< Vertex >, NUM_LAYERS > vertices = {};
   std::array< std::bitset< MAX_SPRITES_PER_LAYER >, NUM_LAYERS > verticesAvail = {};

   std::array< Buffer, NUM_LAYERS > vertexBuffer = {};

   // Index
   std::array< std::vector< IndexType >, NUM_LAYERS > indices = {};
   std::array< Buffer, NUM_LAYERS > indexBuffer = {};

   std::array< uint32_t, NUM_LAYERS > numMeshes = {};
   uint32_t totalNumMeshes = {};


   ////////////////////////////////////
   //                                //
   ////////////////////////////////////

   // SSBO (PerInstanceBuffer)
   std::vector< PerInstanceBuffer > perInstance = {};
   std::array< Buffer, MAX_FRAMES_IN_FLIGHT > ssbo = {};


   // UBO (UniformBufferObject)
   std::vector< Buffer > uniformBuffers{MAX_FRAMES_IN_FLIGHT};

   VkSurfaceKHR surface = VK_NULL_HANDLE;
   GLFWwindow* windowHandle = nullptr;

   // Swapchain
   VkSwapchainKHR swapChain = {};
   VkExtent2D swapChainExtent = {};
   std::vector< VkImage > swapChainImages = {MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE};
   std::vector< VkImageView > swapChainImageViews = {MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE};
   std::vector< VkFramebuffer > swapChainFramebuffers = {MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE};

   // Quads and non UI stuff
   VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
   VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
   std::vector< VkDescriptorSet > descriptorSets = {MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE};

   VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;

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

   glm::mat4 viewMat = {};
   glm::mat4 projMat = {};
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
   inline static VmaAllocator vk_hAllocator;
   inline static VkPhysicalDevice vk_physicalDevice = VK_NULL_HANDLE;
   inline static VkQueue vk_graphicsQueue = {};
   inline static VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

   inline static VkCommandPool commandPool = {};
   inline static std::vector< VkCommandBuffer > commandBuffers = {};

   inline static std::unordered_map< ApplicationType, RenderData > renderData_ = {};

   inline static uint32_t currentFrame_ = {};
};

inline uint32_t
GetNextFrame()
{
   return (Data::currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

struct EditorData
{
   // Pathfinder
   inline static std::vector< Vertex > pathfinderVertices_ = {};
   inline static std::vector< uint32_t > pathfinderIndices_ = {};
   inline static Buffer pathfinderVertexBuffer = {};
   inline static Buffer pathfinderIndexBuffer = {};
   inline static uint32_t numNodes_ = {};

   // Animation
   inline static std::vector< Vertex > animationVertices_ = {};
   inline static std::vector< uint32_t > animationIndices_ = {};
   inline static Buffer animationVertexBuffer = {};
   inline static Buffer animationIndexBuffer = {};
   inline static uint32_t numPoints_ = {};

   // Lines
   inline static VkPipeline linePipeline_ = {};
   inline static VkPipelineLayout linePipelineLayout_ = {};
   inline static VkDescriptorPool lineDescriptorPool = {};
   inline static VkDescriptorSetLayout lineDescriptorSetLayout_ = {};
   inline static std::vector< VkDescriptorSet > lineDescriptorSets_ = {};
   inline static Buffer lineVertexBuffer = {};
   inline static Buffer lineIndexBuffer = {};
   inline static std::vector< LineVertex > lineVertices_ = {};
   inline static std::vector< uint32_t > lineIndices_ = {};
   inline static std::vector< Buffer > lineUniformBuffers_ = {};
   inline static uint32_t numGridLines = {};
   inline static uint32_t numLines = {};
   inline static uint32_t curDynLineIdx = {};

   // UI
   inline static VkImage fontImage_ = {};
   inline static VkDeviceMemory fontMemory_ = {};
   inline static VkImageView m_fontView = {};
   inline static VkSampler sampler_ = {};
   inline static VkDescriptorPool descriptorPool_ = {};
   inline static VkDescriptorSetLayout descriptorSetLayout_ = {};
   inline static VkDescriptorSet descriptorSet_ = {};

   inline static VkPipeline pipeline_ = {};
   inline static VkPipelineLayout pipelineLayout_ = {};
   inline static uint32_t subpass_ = 0;

   inline static PushConstBlock pushConstant_ = {};
   inline static std::vector< renderer::Buffer > vertexBuffer_ = {};
   inline static std::vector< renderer::Buffer > indexBuffer_ = {};
   inline static std::vector< int32_t > vertexCount_ = {};
   inline static std::vector< int32_t > indexCount_ = {};
};

} // namespace looper::renderer
