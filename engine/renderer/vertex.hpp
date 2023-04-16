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
   getBindingDescription()
   {
      VkVertexInputBindingDescription bindingDescription{};
      bindingDescription.binding = 0;
      bindingDescription.stride = sizeof(LineVertex);
      bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescription;
   }

   static auto
   getAttributeDescriptions()
   {
      std::array< VkVertexInputAttributeDescription, 1 > attributeDescriptions{};

      attributeDescriptions[0].binding = 0;
      attributeDescriptions[0].location = 0;
      attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[0].offset = offsetof(LineVertex, m_position);

      return attributeDescriptions;
   }
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
   getBindingDescription()
   {
      VkVertexInputBindingDescription bindingDescription{};
      bindingDescription.binding = 0;
      bindingDescription.stride = sizeof(Vertex);
      bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescription;
   }

   static auto
   getAttributeDescriptions()
   {
      std::array< VkVertexInputAttributeDescription, 2 > attributeDescriptions{};

      attributeDescriptions[0].binding = 0;
      attributeDescriptions[0].location = 0;
      attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[0].offset = offsetof(Vertex, m_position);

      attributeDescriptions[1].binding = 0;
      attributeDescriptions[1].location = 1;
      attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[1].offset = offsetof(Vertex, m_texCoordsDraw);

      return attributeDescriptions;
   }
};

} // namespace looper::renderer
