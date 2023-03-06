#include "vulkan_common.hpp"
#include "utils/assert.hpp"

namespace looper::renderer {

uint32_t
FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
   VkPhysicalDeviceMemoryProperties memProperties = {};
   vkGetPhysicalDeviceMemoryProperties(Data::vk_physicalDevice, &memProperties);

   for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
   {
      if ((typeFilter & (1 << i))
          // NOLINTNEXTLINE
          && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
      {
         return i;
      }
   }

   utils::Assert(false, "failed to find suitable memory type!");
   return 1;
}

} // namespace shady::renderer
