#pragma once

#include <string_view>
#include <utility>
#include <vulkan/vulkan.h>

namespace looper::render {

struct ShaderInfoWrapper
{
   /*
    * This should be called after the pipeline is created
    */
   void
   Destroy()
   {
      vkDestroyShaderModule(device, shaderInfo.module, nullptr);
   }

   VkDevice device;
   VkPipelineShaderStageCreateInfo shaderInfo;
};

using VertexShaderInfo = ShaderInfoWrapper;
using FragmentShaderInfo = ShaderInfoWrapper;

class VulkanShader
{
 public:
   static std::pair< VertexShaderInfo, FragmentShaderInfo >
   CreateShader(VkDevice device, std::string_view vertex, std::string_view fragment);
};

} // namespace looper::render
