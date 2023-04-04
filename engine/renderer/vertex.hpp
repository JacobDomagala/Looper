#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <array>

namespace looper::renderer {

struct Vertex
{
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

} // namespace shady::renderer
