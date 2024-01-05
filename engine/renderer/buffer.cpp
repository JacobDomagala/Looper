#include "buffer.hpp"
#include "command.hpp"
#include "utils/assert.hpp"
#include "vulkan_common.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <fmt/format.h>

namespace looper::renderer {
bool
IsMapped(VmaAllocation alloc)
{
   VmaAllocationInfo allocInfo;
   vmaGetAllocationInfo(Data::vk_hAllocator, alloc, &allocInfo);

   return allocInfo.pMappedData != nullptr;
}

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

void
Buffer::Map()
{
   if (not IsMapped(allocation_))
   {
      vmaMapMemory(Data::vk_hAllocator, allocation_, &mappedMemory_);
      mapped_ = true;
   }
}

void
Buffer::Unmap()
{
   if (IsMapped(allocation_))
   {
      vmaUnmapMemory(Data::vk_hAllocator, allocation_);
      mapped_ = false;
      mappedMemory_ = nullptr;
   }
}

void
Buffer::CopyData(const void* data) const
{
   utils::Assert(mapped_, "Buffer is not mapped!");
   memcpy(mappedMemory_, data, bufferSize_);
}

void
Buffer::CopyDataWithStaging(const void* data, const size_t dataSize) const
{
   auto stagingBuffer = Buffer::CreateBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                                                | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

   void* mapped_data = {};
   vmaMapMemory(Data::vk_hAllocator, allocation_, &mapped_data);
   memcpy(mapped_data, data, dataSize);
   vmaUnmapMemory(Data::vk_hAllocator, allocation_);

   Buffer::CopyBuffer(stagingBuffer.buffer_, buffer_, dataSize);
   stagingBuffer.Destroy();
}

void
Buffer::CopyDataToImageWithStaging(VkImage image, const void* data, const size_t dataSize,
                                   const std::vector< VkBufferImageCopy >& copyRegions)
{
   auto stagingBuffer = Buffer::CreateBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                                                | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

   void* mapped_data = {};
   vmaMapMemory(Data::vk_hAllocator, stagingBuffer.allocation_, &mapped_data);
   memcpy(mapped_data, data, dataSize);
   vmaUnmapMemory(Data::vk_hAllocator, stagingBuffer.allocation_);

   auto* commandBuffer = Command::BeginSingleTimeCommands();

   vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.buffer_, image,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          static_cast< uint32_t >(copyRegions.size()), copyRegions.data());

   Command::EndSingleTimeCommands(commandBuffer);

   stagingBuffer.Destroy();
}

void
Buffer::SetupDescriptor(VkDeviceSize /*size*/, VkDeviceSize offset)
{
   descriptor_.offset = offset;
   descriptor_.buffer = buffer_;
   descriptor_.range = bufferSize_;
}

void
AllocateMemory(VkMemoryRequirements memReq, VkDeviceMemory& bufferMemory,
               VkMemoryPropertyFlags properties)
{
   VkMemoryAllocateInfo allocInfo = {};
   allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   allocInfo.allocationSize = memReq.size;
   allocInfo.memoryTypeIndex = FindMemoryType(memReq.memoryTypeBits, properties);

   vk_check_error(vkAllocateMemory(Data::vk_device, &allocInfo, nullptr, &bufferMemory),
                  "failed to allocate buffer memory!");
}

void
Buffer::AllocateImageMemory(VkImage image, VkDeviceMemory& bufferMemory,
                            VkMemoryPropertyFlags properties)
{
   VkMemoryRequirements memRequirements = {};
   vkGetImageMemoryRequirements(Data::vk_device, image, &memRequirements);

   AllocateMemory(memRequirements, bufferMemory, properties);
}

void
Buffer::AllocateBufferMemory(VkBuffer buffer, VkDeviceMemory& bufferMemory,
                             VkMemoryPropertyFlags properties)
{
   VkMemoryRequirements memRequirements = {};
   vkGetBufferMemoryRequirements(Data::vk_device, buffer, &memRequirements);

   AllocateMemory(memRequirements, bufferMemory, properties);
}

Buffer
Buffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
   Buffer newBuffer = {};
   newBuffer.bufferSize_ = size;

   VkBufferCreateInfo bufferInfo = {};
   bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
   bufferInfo.size = size;
   bufferInfo.usage = usage;
   bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

   VmaAllocationCreateInfo allocInfo = {};
   allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
   
   if (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
   {
      allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
   }

   allocInfo.requiredFlags = properties;

   VmaAllocationInfo allocationInfo;
   vk_check_error(vmaCreateBuffer(Data::vk_hAllocator, &bufferInfo, &allocInfo, &newBuffer.buffer_,
                                  &newBuffer.allocation_, &allocationInfo),
                  "");

  newBuffer.bufferMemory_ = allocationInfo.deviceMemory;

  //  vkBindBufferMemory(Data::vk_device, newBuffer.buffer_, newBuffer.bufferMemory_, 0);
   newBuffer.SetupDescriptor();

   return newBuffer;
}

void
Buffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
   auto* commandBuffer = Command::BeginSingleTimeCommands();

   VkBufferCopy copyRegion = {};
   copyRegion.size = size;
   vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

   Command::EndSingleTimeCommands(commandBuffer);
}

void
Buffer::FreeMemory(VkBuffer& buffer, VkDeviceMemory& memory)
{
   vkDestroyBuffer(Data::vk_device, buffer, nullptr);
   vkFreeMemory(Data::vk_device, memory, nullptr);

   buffer = VK_NULL_HANDLE;
   memory = VK_NULL_HANDLE;
}

void
Buffer::Flush(VkDeviceSize size, VkDeviceSize offset) const
{
   VkMappedMemoryRange mappedRange = {};
   mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
   mappedRange.memory = bufferMemory_;
   mappedRange.offset = offset;
   mappedRange.size = size;

   vk_check_error(vkFlushMappedMemoryRanges(Data::vk_device, 1, &mappedRange),
                  "Buffer::Flush error!");
}

void
Buffer::Destroy()
{
   Unmap();

   vmaDestroyBuffer(Data::vk_hAllocator, buffer_, allocation_);
   buffer_ = VK_NULL_HANDLE;
   bufferMemory_ = VK_NULL_HANDLE;
}

} // namespace looper::renderer
