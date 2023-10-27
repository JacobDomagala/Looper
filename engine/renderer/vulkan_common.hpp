#pragma once

#include "buffer.hpp"
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

static constexpr uint32_t MAX_MESHES_PER_LAYER = 100;
static constexpr uint32_t MAX_NUM_TEXTURES = 256;
static constexpr uint32_t INDICES_PER_SPRITE = 6;
static constexpr uint32_t VERTICES_PER_SPRITE = 4;
static constexpr uint32_t INDICES_PER_LINE = 2;
static constexpr uint32_t VERTICES_PER_LINE = 2;
inline constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 3;
inline constexpr uint32_t MAX_NUM_LINES = 100000;
inline constexpr uint32_t NUM_LAYERS = 11;
static constexpr bool ENABLE_VALIDATION = true;
static constexpr std::array< const char*, 1 > VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};
static constexpr std::array< const char*, 1 > DEVICE_EXTENSIONS = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct Vertex;

struct RenderInfo
{
   uint32_t idx;
   uint32_t layer;
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
   std::array< VkBuffer, NUM_LAYERS > vertexBuffer = {VK_NULL_HANDLE};
   std::array< VkDeviceMemory, NUM_LAYERS > vertexBufferMemory = {VK_NULL_HANDLE};

   // Index
   std::array< std::vector< IndexType >, NUM_LAYERS > indices = {};
   std::array< VkBuffer, NUM_LAYERS > indexBuffer = {VK_NULL_HANDLE};
   std::array< VkDeviceMemory, NUM_LAYERS > indexBufferMemory = {VK_NULL_HANDLE};

   std::array< uint32_t, NUM_LAYERS > numMeshes = {};
   uint32_t totalNumMeshes = {};


   ////////////////////////////////////
   //                                //
   ////////////////////////////////////

   // SSBO (PerInstanceBuffer)
   std::vector< PerInstanceBuffer > perInstance = {};
   std::array< VkBuffer, MAX_FRAMES_IN_FLIGHT > ssbo = {VK_NULL_HANDLE};
   std::array< VkDeviceMemory, MAX_FRAMES_IN_FLIGHT > ssboMemory = {VK_NULL_HANDLE};


   // UBO (UniformBufferObject)
   std::vector< VkBuffer > uniformBuffers = {MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE};
   std::vector< VkDeviceMemory > uniformBuffersMemory = {MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE};


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

   // Lines
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
};

} // namespace looper::renderer
