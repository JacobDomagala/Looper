#pragma once

#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace looper::renderer {

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
   CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

   static void
   FreeMemory(VkBuffer& buffer, VkDeviceMemory& memory);

   void
   Map();

   void
   Unmap();

   void
   CopyData(const void* data) const;

   void
   CopyDataWithStaging(const void* data, const size_t dataSize) const;

   static void
   CopyDataToImageWithStaging(VkImage image, const void* data, const size_t dataSize,
                              const std::vector< VkBufferImageCopy >& copyRegions);

   void
   Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;

   void
   SetupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

   void
   Destroy();

 public:
   void* mappedMemory_ = nullptr;
   bool mapped_ = false;
   VkBuffer buffer_ = {};
   VkDeviceMemory bufferMemory_ = {};
   VkDeviceSize bufferSize_ = {};
   VmaAllocation allocation_ = {};
   VkDescriptorBufferInfo descriptor_ = {};
};


} // namespace looper::renderer
