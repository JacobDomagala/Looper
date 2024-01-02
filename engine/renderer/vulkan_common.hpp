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

static constexpr uint32_t VERTICES_PER_SPRITE = 4;
static constexpr uint32_t MAX_SPRITES_PER_LAYER = 1000;
static constexpr uint32_t MAX_NUM_VERTICES_PER_LAYER = MAX_SPRITES_PER_LAYER * VERTICES_PER_SPRITE;
inline constexpr uint32_t NUM_LAYERS = 11;
static constexpr size_t MAX_NUM_SPRITES = MAX_SPRITES_PER_LAYER * NUM_LAYERS;
static constexpr uint32_t MAX_NUM_TEXTURES = 256;
static constexpr uint32_t INDICES_PER_SPRITE = 6;
static constexpr uint32_t INDICES_PER_LINE = 2;
static constexpr uint32_t VERTICES_PER_LINE = 2;
inline constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 3;
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

   std::array< Buffer, NUM_LAYERS > vertexBuffer = {VK_NULL_HANDLE};

   // Index
   std::array< std::vector< IndexType >, NUM_LAYERS > indices = {};
   std::array< Buffer, NUM_LAYERS > indexBuffer = {VK_NULL_HANDLE};

   std::array< uint32_t, NUM_LAYERS > numMeshes = {};
   uint32_t totalNumMeshes = {};


   ////////////////////////////////////
   //                                //
   ////////////////////////////////////

   // SSBO (PerInstanceBuffer)
   std::vector< PerInstanceBuffer > perInstance = {};
   std::array< Buffer, MAX_FRAMES_IN_FLIGHT > ssbo = {VK_NULL_HANDLE};


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
};

} // namespace looper::renderer
