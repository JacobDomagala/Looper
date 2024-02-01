#pragma once

#include <array>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace looper::renderer {

struct LineVertex
{
   glm::vec3 position_;

   static VkVertexInputBindingDescription
   getBindingDescription();

   static std::array< VkVertexInputAttributeDescription, 1 >
   getAttributeDescriptions();
};

struct Vertex
{
   glm::vec3 position_;
   glm::vec3 texCoordsDraw_; // texcoords and drawID

   static VkVertexInputBindingDescription
   getBindingDescription();

   static std::array< VkVertexInputAttributeDescription, 2 >
   getAttributeDescriptions();
};

} // namespace looper::renderer
