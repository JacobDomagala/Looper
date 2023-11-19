#pragma once

#include <vector>
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
   CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                VkBuffer& buffer, VkDeviceMemory& bufferMemory);

   static void
   CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

   static void
   FreeMemory(VkBuffer& buffer, VkDeviceMemory& memory);

   void
   Map(VkDeviceSize size = VK_WHOLE_SIZE);

   void
   Unmap();

   void
   CopyData(const void* data) const;

   void
   CopyDataWithStaging(void* data, size_t dataSize) const;

   static void
   CopyDataToImageWithStaging(VkImage image, void* data, size_t dataSize,
                              const std::vector< VkBufferImageCopy >& copyRegions);

   void
   Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;

   void
   SetupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

   void
   Destroy();

 public:
   void* m_mappedMemory = nullptr;
   bool m_mapped = false;
   VkBuffer m_buffer = {};
   VkDeviceMemory m_bufferMemory = {};
   VkDeviceSize m_bufferSize = {};
   VkDescriptorBufferInfo m_descriptor = {};
};


} // namespace looper::renderer
