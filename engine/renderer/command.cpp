#include "command.hpp"
#include "vulkan_common.hpp"

namespace looper::render {

VkCommandBuffer
Command::BeginSingleTimeCommands()
{
   VkCommandBufferAllocateInfo allocInfo = {};
   allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   allocInfo.commandPool = Data::vk_commandPool;
   allocInfo.commandBufferCount = 1;

   VkCommandBuffer commandBuffer = {};
   vkAllocateCommandBuffers(Data::vk_device, &allocInfo, &commandBuffer);

   VkCommandBufferBeginInfo beginInfo = {};
   beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

   vkBeginCommandBuffer(commandBuffer, &beginInfo);

   return commandBuffer;
}

void
Command::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
   vkEndCommandBuffer(commandBuffer);

   VkSubmitInfo submitInfo = {};
   submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submitInfo.commandBufferCount = 1;
   submitInfo.pCommandBuffers = &commandBuffer;

   vkQueueSubmit(Data::vk_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
   vkQueueWaitIdle(Data::vk_graphicsQueue);

   vkFreeCommandBuffers(Data::vk_device, Data::vk_commandPool, 1, &commandBuffer);
}

} // namespace looper::render
