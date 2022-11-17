#pragma once

#include "utils/assert.hpp"

#include <vulkan/vulkan.h>
#include <array>
#include <vulkan/vulkan_core.h>

#define VK_CHECK(vkFunction, errorMessage)                                              \
   do                                                                                   \
   {                                                                                    \
      const auto result = vkFunction;                                                   \
      if (result != VK_SUCCESS)                                                         \
      {                                                                                 \
         utils::Assert(false, fmt::format("{} Return value {}", errorMessage, result)); \
      }                                                                                 \
   } while (0);


namespace looper::render::vulkan {



static constexpr bool ENABLE_VALIDATION = true;
static constexpr std::array< const char*, 1 > VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};
static constexpr std::array< const char*, 1 > DEVICE_EXTENSIONS = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

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
};

uint32_t
FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

} // namespace looper::render::vulkan
