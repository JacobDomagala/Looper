#pragma once

#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace looper::renderer {

struct LineVertex
{
   struct PushConstants
   {
      glm::vec4 color = {};
   };
   static inline constexpr bool HAS_PUSHCONSTANTS = true;
   static inline constexpr VkShaderStageFlags SHADER_STAGE_FLAGS = VK_SHADER_STAGE_FRAGMENT_BIT;
   static inline constexpr VkPrimitiveTopology TOPOLOGY = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

   glm::vec3 m_position;

   static VkVertexInputBindingDescription
   getBindingDescription();

   static std::array< VkVertexInputAttributeDescription, 1 >
   getAttributeDescriptions();

   static void
   CreateDescriptorPool();

   static void
   CreateDescriptorSetLayout();

   static void
   CreateDescriptorSets();
};

struct Vertex
{
   struct PushConstants
   {
      float selectedIdx = {};
   };
   static inline constexpr bool HAS_PUSHCONSTANTS = true;
   static inline constexpr VkShaderStageFlags SHADER_STAGE_FLAGS = VK_SHADER_STAGE_VERTEX_BIT;
   static inline constexpr VkPrimitiveTopology TOPOLOGY = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

   glm::vec3 m_position;
   glm::vec3 m_texCoordsDraw; // texcoords and drawID

   static VkVertexInputBindingDescription
   getBindingDescription();

   static std::array< VkVertexInputAttributeDescription, 2 >
   getAttributeDescriptions();

   static void
   CreateDescriptorPool();

   static void
   CreateDescriptorSetLayout();

   static void
   CreateDescriptorSets();

   static void
   UpdateDescriptorSets();
};

} // namespace looper::renderer
