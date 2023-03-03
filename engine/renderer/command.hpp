#pragma once

#include <vulkan/vulkan.h>

namespace looper::render {

class Command
{
 public:
   static VkCommandBuffer
   BeginSingleTimeCommands();

   static void
   EndSingleTimeCommands(VkCommandBuffer commandBuffer);
};

} // namespace looper::render
