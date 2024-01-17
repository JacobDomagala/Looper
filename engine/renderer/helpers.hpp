#pragma once

#include "logger/logger.hpp"
#include "utils/assert.hpp"
#include "vulkan_common.hpp"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <optional>
#include <set>

namespace looper::renderer {

struct SwapChainSupportDetails
{
   VkSurfaceCapabilitiesKHR capabilities = {};
   std::vector< VkSurfaceFormatKHR > formats = {};
   std::vector< VkPresentModeKHR > presentModes = {};
};

struct QueueFamilyIndices
{
   uint32_t graphicsFamily = UINT32_MAX;
   uint32_t presentFamily = UINT32_MAX;

   [[nodiscard]] bool
   IsComplete() const
   {
      return graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX;
   }
};

inline std::set< std::string >
GetSupportedExtensions()
{
   uint32_t count = {};
   vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
   std::vector< VkExtensionProperties > extensions(count);
   vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
   std::set< std::string > results = {};
   for (auto& extension : extensions)
   {
      results.insert(&extension.extensionName[0]);
   }
   return results;
}

inline std::vector< const char* >
GetRequiredExtensions()
{
   uint32_t glfwExtensionCount = 0;
   const auto** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

   std::vector< const char* > extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

   if constexpr (ENABLE_VALIDATION)
   {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
   }

   return extensions;
}

inline bool
CheckValidationLayerSupport()
{
   uint32_t layerCount = {};
   vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

   std::vector< VkLayerProperties > availableLayers(layerCount);
   vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

   std::set< std::string > validationLayers(VALIDATION_LAYERS.begin(), VALIDATION_LAYERS.end());

   for (const auto& extension : availableLayers)
   {
      validationLayers.erase(&extension.layerName[0]);
   }

   return validationLayers.empty();
}

inline void
PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
   auto callback = [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                      VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
                      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                      void* /*pUserData*/) -> VKAPI_ATTR VkBool32 {
      switch (messageSeverity)
      {
         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
            Logger::Debug("validation layer: {}", pCallbackData->pMessage);
         }
         break;

         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
            Logger::Info("validation layer: {}", pCallbackData->pMessage);
         }
         break;

         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
            Logger::Warn("validation layer: {}", pCallbackData->pMessage);
         }
         break;

         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
         default: {
            // utils::Assert(false, fmt::format("validation layer: {}", pCallbackData->pMessage));
            Logger::Fatal("validation layer: {}", pCallbackData->pMessage);
         }
      }

      return VK_FALSE;
   };

   createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
   createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
   createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
   createInfo.pfnUserCallback = callback;
}

inline QueueFamilyIndices
FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
   QueueFamilyIndices indices_ = {};

   uint32_t queueFamilyCount = 0;
   vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

   std::vector< VkQueueFamilyProperties > queueFamilies(queueFamilyCount);
   vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

   for (uint32_t index = 0; const auto& queueFamily : queueFamilies)
   {
      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
      {
         indices_.graphicsFamily = index;
      }

      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &presentSupport);

      if (presentSupport)
      {
         indices_.presentFamily = index;
      }

      if (indices_.IsComplete())
      {
         break;
      }

      index++;
   }

   return indices_;
}

inline SwapChainSupportDetails
QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
   SwapChainSupportDetails details = {};

   vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

   uint32_t formatCount = {};
   vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

   if (formatCount != 0)
   {
      details.formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
   }

   uint32_t presentModeCount = {};
   vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

   if (presentModeCount != 0)
   {
      details.presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
                                                details.presentModes.data());
   }

   return details;
}

inline bool
CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
   uint32_t extensionCount = {};
   vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

   std::vector< VkExtensionProperties > availableExtensions(extensionCount);
   vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                        availableExtensions.data());

   std::set< std::string > requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

   for (const auto& extension : availableExtensions)
   {
      requiredExtensions.erase(&extension.extensionName[0]);
   }

   return requiredExtensions.empty();
}

inline bool
IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
   static_cast< void >(FindQueueFamilies(device, surface));

   const auto extensionsSupported = CheckDeviceExtensionSupport(device);

   bool swapChainAdequate = false;
   if (extensionsSupported)
   {
      const auto swapChainSupport = QuerySwapChainSupport(device, surface);
      swapChainAdequate =
         !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
   }

   VkPhysicalDeviceFeatures supportedFeatures = {};
   vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

   // Make sure we use discrete GPU
   VkPhysicalDeviceProperties physicalDeviceProperties = {};
   vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);
   auto isDiscrete = physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

   return extensionsSupported && swapChainAdequate && isDiscrete
          && supportedFeatures.samplerAnisotropy && supportedFeatures.multiDrawIndirect
          && supportedFeatures.wideLines;
}

inline VkSampleCountFlagBits
GetMaxUsableSampleCount(VkPhysicalDevice& physicalDevice)
{
   VkPhysicalDeviceProperties physicalDeviceProperties = {};
   vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

   const VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts
                                     & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
   if (counts & VK_SAMPLE_COUNT_64_BIT)
   {
      return VK_SAMPLE_COUNT_64_BIT;
   }
   if (counts & VK_SAMPLE_COUNT_32_BIT)
   {
      return VK_SAMPLE_COUNT_32_BIT;
   }
   if (counts & VK_SAMPLE_COUNT_16_BIT)
   {
      return VK_SAMPLE_COUNT_16_BIT;
   }
   if (counts & VK_SAMPLE_COUNT_8_BIT)
   {
      return VK_SAMPLE_COUNT_8_BIT;
   }
   if (counts & VK_SAMPLE_COUNT_4_BIT)
   {
      return VK_SAMPLE_COUNT_4_BIT;
   }
   if (counts & VK_SAMPLE_COUNT_2_BIT)
   {
      return VK_SAMPLE_COUNT_2_BIT;
   }

   return VK_SAMPLE_COUNT_1_BIT;
}

inline VkSurfaceFormatKHR
ChooseSwapSurfaceFormat(const std::vector< VkSurfaceFormatKHR >& availableFormats)
{
   const auto format = std::ranges::find_if(availableFormats, [](const auto& availableFormat) {
      return availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
             && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
   });


   return format != availableFormats.end() ? *format : availableFormats[0];
}

inline VkPresentModeKHR
ChooseSwapPresentMode(const std::vector< VkPresentModeKHR >& availablePresentModes)
{
   const auto mode =
      std::ranges::find_if(availablePresentModes, [](const auto& availablePresentMode) {
         return availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR;
      });


   return mode != availablePresentModes.end() ? *mode : VK_PRESENT_MODE_FIFO_KHR;
}

inline VkExtent2D
ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* windowHandle)
{
   if (capabilities.currentExtent.width != UINT32_MAX and capabilities.currentExtent.width > 0
       and capabilities.currentExtent.height != UINT32_MAX
       and capabilities.currentExtent.height > 0)
   {
      return capabilities.currentExtent;
   }

   int32_t width = {};
   int32_t height = {};
   glfwGetFramebufferSize(windowHandle, &width, &height);

   VkExtent2D actualExtent = {static_cast< uint32_t >(width), static_cast< uint32_t >(height)};

   actualExtent.width = glm::clamp(actualExtent.width, capabilities.maxImageExtent.width,
                                   capabilities.minImageExtent.width);

   actualExtent.height = glm::clamp(actualExtent.height, capabilities.maxImageExtent.height,
                                    capabilities.minImageExtent.height);

   return actualExtent;
}

VkFormat
FindSupportedFormat(const std::vector< VkFormat >& candidates, VkImageTiling tiling,
                    VkFormatFeatureFlags features)
{
   for (const auto format : candidates)
   {
      VkFormatProperties props = {};
      vkGetPhysicalDeviceFormatProperties(Data::vk_physicalDevice, format, &props);
      const auto tiling_linear =
         tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features;
      const auto tiling_optimal =
         tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features;

      if (tiling_linear or tiling_optimal)
      {
         return format;
      }
   }

   utils::Assert(false, "failed to find supported format!");
   return {};
}

VkFormat
FindDepthFormat()
{
   return FindSupportedFormat({VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                              VK_IMAGE_TILING_OPTIMAL,
                              VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

} // namespace looper::renderer
