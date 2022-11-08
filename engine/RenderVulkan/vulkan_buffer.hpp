#pragma once

#include <vulkan/vulkan.h>

namespace looper::render::vulkan {

class Buffer
{
 public:
   static void
   AllocateImageMemory(VkImage image, VkDeviceMemory& bufferMemory,
                       VkMemoryPropertyFlags properties);

   static void
   AllocateBufferMemory(VkBuffer buffer, VkDeviceMemory& bufferMemory,
                        VkMemoryPropertyFlags properties);

   static Buffer
   CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

   static void
   CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                VkBuffer& buffer, VkDeviceMemory& bufferMemory);

   static void
   CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

 public:
   VkBuffer m_buffer = {};
   VkDeviceMemory m_bufferMemory = {};
};


} // namespace looper::render::vulkan
