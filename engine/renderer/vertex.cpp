#include "vertex.hpp"
#include "vulkan_common.hpp"

namespace looper::renderer {

VkVertexInputBindingDescription
LineVertex::getBindingDescription()
{
   VkVertexInputBindingDescription bindingDescription{};
   bindingDescription.binding = 0;
   bindingDescription.stride = sizeof(LineVertex);
   bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

   return bindingDescription;
}

std::array< VkVertexInputAttributeDescription, 1 >
LineVertex::getAttributeDescriptions()
{
   std::array< VkVertexInputAttributeDescription, 1 > attributeDescriptions{};

   attributeDescriptions[0].binding = 0;
   attributeDescriptions[0].location = 0;
   attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
   attributeDescriptions[0].offset = offsetof(LineVertex, position_);

   return attributeDescriptions;
}

VkVertexInputBindingDescription
Vertex::getBindingDescription()
{
   VkVertexInputBindingDescription bindingDescription{};
   bindingDescription.binding = 0;
   bindingDescription.stride = sizeof(Vertex);
   bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

   return bindingDescription;
}

std::array< VkVertexInputAttributeDescription, 2 >
Vertex::getAttributeDescriptions()
{
   std::array< VkVertexInputAttributeDescription, 2 > attributeDescriptions{};

   attributeDescriptions[0].binding = 0;
   attributeDescriptions[0].location = 0;
   attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
   attributeDescriptions[0].offset = offsetof(Vertex, position_);

   attributeDescriptions[1].binding = 0;
   attributeDescriptions[1].location = 1;
   attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
   attributeDescriptions[1].offset = offsetof(Vertex, texCoordsDraw_);

   return attributeDescriptions;
}

} // namespace looper::renderer
