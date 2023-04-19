#pragma once

#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace looper::renderer {

struct LineVertex
{
   glm::vec3 m_position;

   static VkVertexInputBindingDescription
   getBindingDescription();

   static std::array< VkVertexInputAttributeDescription, 1 >
   getAttributeDescriptions();
};

struct Vertex
{
   glm::vec3 m_position;
   glm::vec3 m_texCoordsDraw; // texcoords and drawID

   static VkVertexInputBindingDescription
   getBindingDescription();

   static std::array< VkVertexInputAttributeDescription, 2 >
   getAttributeDescriptions();
};

} // namespace looper::renderer
