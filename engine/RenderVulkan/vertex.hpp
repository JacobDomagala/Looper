#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>

namespace looper::render::vulkan {

struct Vertex
{
   glm::vec3 m_position;
   glm::vec3 m_normal;
   glm::vec2 m_texCoords;
   glm::vec3 m_tangent;
   glm::vec4 m_color;

   static VkVertexInputBindingDescription
   getBindingDescription()
   {
      VkVertexInputBindingDescription bindingDescription{};
      bindingDescription.binding = 0;
      bindingDescription.stride = sizeof(Vertex);
      bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescription;
   }

   static std::array< VkVertexInputAttributeDescription, 5 >
   getAttributeDescriptions()
   {
      std::array< VkVertexInputAttributeDescription, 5 > attributeDescriptions{};

      attributeDescriptions[0].binding = 0;
      attributeDescriptions[0].location = 0;
      attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[0].offset = offsetof(Vertex, m_position);

      attributeDescriptions[1].binding = 0;
      attributeDescriptions[1].location = 1;
      attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[1].offset = offsetof(Vertex, m_normal);

      attributeDescriptions[2].binding = 0;
      attributeDescriptions[2].location = 2;
      attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
      attributeDescriptions[2].offset = offsetof(Vertex, m_texCoords);

      attributeDescriptions[3].binding = 0;
      attributeDescriptions[3].location = 3;
      attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[3].offset = offsetof(Vertex, m_tangent);

      attributeDescriptions[4].binding = 0;
      attributeDescriptions[4].location = 4;
      attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
      attributeDescriptions[4].offset = offsetof(Vertex, m_color);

      return attributeDescriptions;
   }
};

} // namespace looper::render
