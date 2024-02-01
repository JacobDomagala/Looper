#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <string_view>
#include <utility>

namespace looper::renderer {

struct ShaderInfoWrapper
{
   /*
    * This should be called after the pipeline is created
    */
   void
   Destroy() const
   {
      vkDestroyShaderModule(device, shaderInfo.module, nullptr);
   }

   VkDevice device = {};
   VkPipelineShaderStageCreateInfo shaderInfo = {};
};

using VertexShaderInfo = ShaderInfoWrapper;
using FragmentShaderInfo = ShaderInfoWrapper;

struct VulkanShader
{
   struct PushConstants
   {
      glm::vec4 color = {};
   };

   static std::pair< VertexShaderInfo, FragmentShaderInfo >
   CreateShader(VkDevice device, std::string_view vertex, std::string_view fragment);
};

struct QuadShader
{
   static inline constexpr bool HAS_PUSHCONSTANTS = true;
   static inline constexpr VkShaderStageFlags SHADER_STAGE_FLAGS = VK_SHADER_STAGE_VERTEX_BIT;
   static inline constexpr VkPrimitiveTopology TOPOLOGY = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

   struct PushConstants
   {
      float selectedIdx = {};
      float meshType = {};
   };

   static void
   CreateDescriptorPool();

   static void
   CreateDescriptorSetLayout();

   static void
   CreateDescriptorSets();

   static void
   UpdateDescriptorSets();
};

struct LineShader
{
   static inline constexpr bool HAS_PUSHCONSTANTS = true;
   static inline constexpr VkShaderStageFlags SHADER_STAGE_FLAGS = VK_SHADER_STAGE_FRAGMENT_BIT;
   static inline constexpr VkPrimitiveTopology TOPOLOGY = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

   struct PushConstants
   {
      glm::vec4 color = {};
   };

   static void
   CreateDescriptorPool();

   static void
   CreateDescriptorSetLayout();

   static void
   CreateDescriptorSets();
};

} // namespace looper::renderer
