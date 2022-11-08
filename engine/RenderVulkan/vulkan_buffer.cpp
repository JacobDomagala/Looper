#include "vulkan_buffer.hpp"
#include "utils/assert.hpp"
#include "vulkan_command.hpp"
#include "vulkan_common.hpp"


#include <fmt/format.h>

namespace looper::render::vulkan {

static void
AllocateMemory(VkMemoryRequirements memReq, VkDeviceMemory& bufferMemory,
               VkMemoryPropertyFlags properties)
{
   VkMemoryAllocateInfo allocInfo{};
   allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   allocInfo.allocationSize = memReq.size;
   allocInfo.memoryTypeIndex = FindMemoryType(memReq.memoryTypeBits, properties);

   VK_CHECK(vkAllocateMemory(Data::vk_device, &allocInfo, nullptr, &bufferMemory),
            "failed to allocate buffer memory!");
}

void
Buffer::AllocateImageMemory(VkImage image, VkDeviceMemory& bufferMemory,
                            VkMemoryPropertyFlags properties)
{
   VkMemoryRequirements memRequirements;
   vkGetImageMemoryRequirements(Data::vk_device, image, &memRequirements);

   AllocateMemory(memRequirements, bufferMemory, properties);
}

void
Buffer::AllocateBufferMemory(VkBuffer buffer, VkDeviceMemory& bufferMemory,
                             VkMemoryPropertyFlags properties)
{
   VkMemoryRequirements memRequirements;
   vkGetBufferMemoryRequirements(Data::vk_device, buffer, &memRequirements);

   AllocateMemory(memRequirements, bufferMemory, properties);
}

Buffer
Buffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
   Buffer newBuffer;
   CreateBuffer(size, usage, properties, newBuffer.m_buffer, newBuffer.m_bufferMemory);
   
   return newBuffer;
}

void
Buffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                     VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
   VkBufferCreateInfo bufferInfo{};
   bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
   bufferInfo.size = size;
   bufferInfo.usage = usage;
   bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

   VK_CHECK(vkCreateBuffer(Data::vk_device, &bufferInfo, nullptr, &buffer), "failed to create buffer!");

   AllocateBufferMemory(buffer, bufferMemory, properties);

   vkBindBufferMemory(Data::vk_device, buffer, bufferMemory, 0);
}

void
Buffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
   VkCommandBuffer commandBuffer = Command::BeginSingleTimeCommands();

   VkBufferCopy copyRegion{};
   copyRegion.size = size;
   vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

   Command::EndSingleTimeCommands(commandBuffer);
}

} // namespace looper::render::vulkan
