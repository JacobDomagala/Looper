#pragma once

#include <vulkan/vulkan.h>

namespace looper::renderer {

class Command
{
 public:
   static VkCommandBuffer
   BeginSingleTimeCommands();

   static void
   EndSingleTimeCommands(VkCommandBuffer commandBuffer);
};

} // namespace shady::renderer
