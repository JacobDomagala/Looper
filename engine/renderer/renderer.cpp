
#include "renderer.hpp"
#include "application.hpp"
#include "buffer.hpp"
#include "command.hpp"
#include "logger/logger.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "utils/assert.hpp"
#include "utils/file_manager.hpp"
#include "vulkan_common.hpp"

#include <GLFW/glfw3.h>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <optional>
#include <set>

#undef max
#undef min

namespace looper::renderer {

struct SwapChainSupportDetails
{
   VkSurfaceCapabilitiesKHR capabilities = {};
   std::vector< VkSurfaceFormatKHR > formats = {};
   std::vector< VkPresentModeKHR > presentModes = {};
};

struct QueueFamilyIndices
{
   std::optional< uint32_t > graphicsFamily = {};
   std::optional< uint32_t > presentFamily = {};

   [[nodiscard]] uint32_t
   GetGraphics() const
   {
      utils::Assert(graphicsFamily.has_value(),
                    "QueueFamilyIndices::GetGraphics: graphicsFamily not initialized!");
      return graphicsFamily.value();
   }

   [[nodiscard]] uint32_t
   GetPresent() const
   {
      utils::Assert(presentFamily.has_value(),
                    "QueueFamilyIndices::GetPresent: presentFamily not initialized!");
      return presentFamily.value();
   }

   [[nodiscard]] bool
   isComplete() const
   {
      return graphicsFamily.has_value() && presentFamily.has_value();
   }
};

template < typename ShaderType, typename VertexType >
void
CreatePipeline(std::string_view vertexShader, std::string_view fragmentShader, PrimitiveType type)
{
   auto& renderData = Data::renderData_.at(VulkanRenderer::GetCurrentlyBoundType());

   // Create switch in future if we support more types
   auto& pipeLineLayout =
      type == PrimitiveType::LINE ? Data::linePipelineLayout_ : renderData.pipelineLayout;

   auto& pipeline = type == PrimitiveType::LINE ? Data::linePipeline_ : renderData.pipeline;

   auto& descriptorSetLayout =
      type == PrimitiveType::LINE ? Data::lineDescriptorSetLayout_ : renderData.descriptorSetLayout;

   auto [vertexInfo, fragmentInfo] =
      VulkanShader::CreateShader(Data::vk_device, vertexShader, fragmentShader);
   auto shaderStages = std::to_array({vertexInfo.shaderInfo, fragmentInfo.shaderInfo});

   auto bindingDescription = VertexType::getBindingDescription();
   auto attributeDescriptions = VertexType::getAttributeDescriptions();

   VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
   vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
   vertexInputInfo.vertexBindingDescriptionCount = 1;
   vertexInputInfo.vertexAttributeDescriptionCount =
      static_cast< uint32_t >(attributeDescriptions.size());
   vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
   vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

   VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
   inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
   inputAssembly.topology = ShaderType::TOPOLOGY;
   inputAssembly.primitiveRestartEnable = VK_FALSE;

   VkViewport viewport = {};
   viewport.x = 0.0f;
   viewport.y = 0.0f;
   viewport.width = static_cast< float >(renderData.swapChainExtent.width);
   viewport.height = static_cast< float >(renderData.swapChainExtent.height);
   viewport.minDepth = 0.0f;
   viewport.maxDepth = 1.0f;

   VkRect2D scissor = {};
   scissor.offset = {0, 0};
   scissor.extent = renderData.swapChainExtent;

   VkPipelineViewportStateCreateInfo viewportState = {};
   viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
   viewportState.viewportCount = 1;
   viewportState.pViewports = &viewport;
   viewportState.scissorCount = 1;
   viewportState.pScissors = &scissor;

   VkPipelineRasterizationStateCreateInfo rasterizer = {};
   rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
   rasterizer.depthClampEnable = VK_FALSE;
   rasterizer.rasterizerDiscardEnable = VK_FALSE;
   rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
   rasterizer.lineWidth = 1.0f;
   rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
   // rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
   rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
   rasterizer.depthBiasEnable = VK_FALSE;

   VkPipelineMultisampleStateCreateInfo multisampling = {};
   multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
   multisampling.sampleShadingEnable = VK_FALSE;
   multisampling.rasterizationSamples = renderer::Data::msaaSamples;

   // Define depth and stencil state
   VkStencilOpState stencilOp{};
   stencilOp.failOp = VK_STENCIL_OP_KEEP;
   stencilOp.passOp = VK_STENCIL_OP_KEEP;
   stencilOp.depthFailOp = VK_STENCIL_OP_KEEP;
   stencilOp.compareOp = VK_COMPARE_OP_ALWAYS;
   stencilOp.compareMask = 0xFF;
   stencilOp.writeMask = 0xFF;
   stencilOp.reference = 0x01;

   VkPipelineDepthStencilStateCreateInfo depthStencil = {};
   depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
   depthStencil.depthTestEnable = VK_TRUE;
   depthStencil.depthWriteEnable = VK_TRUE;
   depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
   depthStencil.depthBoundsTestEnable = VK_FALSE;
   depthStencil.minDepthBounds = 0.0f;
   depthStencil.maxDepthBounds = 1.0f;
   depthStencil.stencilTestEnable = VK_TRUE;
   depthStencil.front = stencilOp;
   depthStencil.back = stencilOp;

   VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
   colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                         | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
   colorBlendAttachment.blendEnable = VK_TRUE;
   colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
   colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
   colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
   colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
   colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
   colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

   VkPipelineColorBlendStateCreateInfo colorBlending = {};
   colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
   colorBlending.logicOpEnable = VK_FALSE;
   colorBlending.logicOp = VK_LOGIC_OP_COPY;
   colorBlending.attachmentCount = 1;
   colorBlending.pAttachments = &colorBlendAttachment;
   colorBlending.blendConstants[0] = 0.0f;
   colorBlending.blendConstants[1] = 0.0f;
   colorBlending.blendConstants[2] = 0.0f;
   colorBlending.blendConstants[3] = 0.0f;

   VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
   pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   pipelineLayoutInfo.setLayoutCount = 1;
   pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

   if constexpr (ShaderType::HAS_PUSHCONSTANTS)
   {
      // Define push constant range
      VkPushConstantRange pushConstantRange{};
      pushConstantRange.stageFlags = ShaderType::SHADER_STAGE_FLAGS;
      pushConstantRange.offset = 0;
      pushConstantRange.size = sizeof(ShaderType::PushConstants);

      pipelineLayoutInfo.pushConstantRangeCount = 1;
      pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
   }


   vk_check_error(
      vkCreatePipelineLayout(Data::vk_device, &pipelineLayoutInfo, nullptr, &pipeLineLayout),
      "failed to create pipeline layout!");

   VkGraphicsPipelineCreateInfo pipelineInfo{};
   pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
   pipelineInfo.stageCount = 2;
   pipelineInfo.pStages = shaderStages.data();
   pipelineInfo.pVertexInputState = &vertexInputInfo;
   pipelineInfo.pInputAssemblyState = &inputAssembly;
   pipelineInfo.pViewportState = &viewportState;
   pipelineInfo.pRasterizationState = &rasterizer;
   pipelineInfo.pMultisampleState = &multisampling;
   pipelineInfo.pDepthStencilState = &depthStencil;
   pipelineInfo.pColorBlendState = &colorBlending;
   pipelineInfo.layout = pipeLineLayout;
   pipelineInfo.renderPass = renderData.renderPass;
   pipelineInfo.subpass = 0;
   pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;


   vk_check_error(vkCreateGraphicsPipelines(Data::vk_device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                            nullptr, &pipeline),
                  "failed to create graphics pipeline!");

   // Shader info can be destroyed after the pipeline is created
   vertexInfo.Destroy();
   fragmentInfo.Destroy();
}


std::set< std::string >
get_supported_extensions()
{
   uint32_t count = {};
   vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr); // get number of extensions
   std::vector< VkExtensionProperties > extensions(count);
   vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()); // populate buffer
   std::set< std::string > results = {};
   for (auto& extension : extensions)
   {
      results.insert(&extension.extensionName[0]);
   }
   return results;
}

/*
 *  Query GLFW for required extensions
 */
std::vector< const char* >
getRequiredExtensions()
{
   uint32_t glfwExtensionCount = 0;
   const auto** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

   std::vector< const char* > extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

   if constexpr (ENABLE_VALIDATION)
   {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
   }

   return extensions;
}

bool
checkValidationLayerSupport()
{
   uint32_t layerCount = {};
   vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

   std::vector< VkLayerProperties > availableLayers(layerCount);
   vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

   std::set< std::string > validationLayers(VALIDATION_LAYERS.begin(), VALIDATION_LAYERS.end());

   for (const auto& extension : availableLayers)
   {
      validationLayers.erase(&extension.layerName[0]);
   }

   return validationLayers.empty();
}

void
populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
   auto callback = [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                      VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
                      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                      void* /*pUserData*/) -> VKAPI_ATTR VkBool32 {
      switch (messageSeverity)
      {
         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
            Logger::Debug("validation layer: {}", pCallbackData->pMessage);
         }
         break;

         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
            Logger::Info("validation layer: {}", pCallbackData->pMessage);
         }
         break;

         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
            Logger::Warn("validation layer: {}", pCallbackData->pMessage);
         }
         break;

         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
            Logger::Fatal("validation layer: {}", pCallbackData->pMessage);
         }
         break;

         default: {
            Logger::Fatal("validation layer: {}", pCallbackData->pMessage);
         }
      }

      return VK_FALSE;
   };

   createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
   createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
   createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
   createInfo.pfnUserCallback = callback;
}

QueueFamilyIndices
findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
   QueueFamilyIndices indices_ = {};

   uint32_t queueFamilyCount = 0;
   vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

   std::vector< VkQueueFamilyProperties > queueFamilies(queueFamilyCount);
   vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

   int i = 0;
   for (const auto& queueFamily : queueFamilies)
   {
      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
      {
         indices_.graphicsFamily = i;
      }

      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, static_cast< uint32_t >(i), surface,
                                           &presentSupport);

      if (presentSupport)
      {
         indices_.presentFamily = i;
      }

      if (indices_.isComplete())
      {
         break;
      }

      i++;
   }

   utils::Assert(indices_.isComplete(), "findQueueFamilies: indices_ are not complete!");
   return indices_;
}

SwapChainSupportDetails
querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
   SwapChainSupportDetails details = {};

   vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

   uint32_t formatCount = {};
   vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

   if (formatCount != 0)
   {
      details.formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
   }

   uint32_t presentModeCount = {};
   vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

   if (presentModeCount != 0)
   {
      details.presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
                                                details.presentModes.data());
   }

   return details;
}

bool
checkDeviceExtensionSupport(VkPhysicalDevice device)
{
   uint32_t extensionCount = {};
   vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

   std::vector< VkExtensionProperties > availableExtensions(extensionCount);
   vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                        availableExtensions.data());

   std::set< std::string > requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

   for (const auto& extension : availableExtensions)
   {
      requiredExtensions.erase(&extension.extensionName[0]);
   }

   return requiredExtensions.empty();
}

bool
isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
   static_cast< void >(findQueueFamilies(device, surface));

   const auto extensionsSupported = checkDeviceExtensionSupport(device);

   bool swapChainAdequate = false;
   if (extensionsSupported)
   {
      const auto swapChainSupport = querySwapChainSupport(device, surface);
      swapChainAdequate =
         !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
   }

   VkPhysicalDeviceFeatures supportedFeatures = {};
   vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

   // Make sure we use discrete GPU
   VkPhysicalDeviceProperties physicalDeviceProperties = {};
   vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);
   auto isDiscrete = physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

   return extensionsSupported && swapChainAdequate && isDiscrete
          && supportedFeatures.samplerAnisotropy && supportedFeatures.multiDrawIndirect;
}

VkSampleCountFlagBits
getMaxUsableSampleCount(VkPhysicalDevice& physicalDevice)
{
   VkPhysicalDeviceProperties physicalDeviceProperties = {};
   vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

   const VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts
                                     & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
   if (counts & VK_SAMPLE_COUNT_64_BIT)
   {
      return VK_SAMPLE_COUNT_64_BIT;
   }
   if (counts & VK_SAMPLE_COUNT_32_BIT)
   {
      return VK_SAMPLE_COUNT_32_BIT;
   }
   if (counts & VK_SAMPLE_COUNT_16_BIT)
   {
      return VK_SAMPLE_COUNT_16_BIT;
   }
   if (counts & VK_SAMPLE_COUNT_8_BIT)
   {
      return VK_SAMPLE_COUNT_8_BIT;
   }
   if (counts & VK_SAMPLE_COUNT_4_BIT)
   {
      return VK_SAMPLE_COUNT_4_BIT;
   }
   if (counts & VK_SAMPLE_COUNT_2_BIT)
   {
      return VK_SAMPLE_COUNT_2_BIT;
   }

   return VK_SAMPLE_COUNT_1_BIT;
}

VkSurfaceFormatKHR
chooseSwapSurfaceFormat(const std::vector< VkSurfaceFormatKHR >& availableFormats)
{
   const auto format = std::find_if(
      availableFormats.begin(), availableFormats.end(), [](const auto& availableFormat) {
         return availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
                && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      });


   return format != availableFormats.end() ? *format : availableFormats[0];
}

VkPresentModeKHR
chooseSwapPresentMode(const std::vector< VkPresentModeKHR >& availablePresentModes)
{
   const auto mode = std::find_if(availablePresentModes.begin(), availablePresentModes.end(),
                                  [](const auto& availablePresentMode) {
                                     return availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR;
                                  });


   return mode != availablePresentModes.end() ? *mode : VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* windowHandle)
{
   if (capabilities.currentExtent.width != UINT32_MAX)
   {
      return capabilities.currentExtent;
   }

   int32_t width = {};
   int32_t height = {};
   glfwGetFramebufferSize(windowHandle, &width, &height);

   VkExtent2D actualExtent = {static_cast< uint32_t >(width), static_cast< uint32_t >(height)};

   actualExtent.width = glm::clamp(actualExtent.width, capabilities.maxImageExtent.width,
                                   capabilities.minImageExtent.width);

   actualExtent.height = glm::clamp(actualExtent.height, capabilities.maxImageExtent.height,
                                    capabilities.minImageExtent.height);

   return actualExtent;
}


void
VulkanRenderer::UpdateDescriptors()
{
   updateDescriptors_ = true;
}

uint32_t
VulkanRenderer::MeshLoaded(const std::vector< Vertex >& vertices_in, const TextureMaps& textures_in,
                           const glm::mat4& modelMat, const glm::vec4& color, ObjectType type)
{
   auto* renderData = &Data::renderData_[boundApplication_];
   auto* vertices = &renderData->vertices;
   auto* numObjects = &renderData->numMeshes;
   if (boundApplication_ == ApplicationType::EDITOR)
   {
      switch (type)
      {
         case ObjectType::ANIMATION_POINT: {
            vertices = &EditorData::animationVertices_;
            ++EditorData::numPoints_;
         }
         break;
         case ObjectType::PATHFINDER_NODE: {
            vertices = &EditorData::pathfinderVertices_;
            ++EditorData::numNodes_;
         }
         break;
         default: {
         }
      }
   }

   std::copy(vertices_in.begin(), vertices_in.end(), std::back_inserter(*vertices));
   std::transform(vertices->end() - 4, vertices->end(), vertices->end() - 4,
                  [numObjects](auto& vtx) {
                     vtx.m_texCoordsDraw.z = static_cast< float >(*numObjects);
                     return vtx;
                  });

   // Indices are handled in init
   // std::copy(indicies_in.begin(), indicies_in.end(), std::back_inserter(indices));

   // TODO: If we go back to indirect draw, this also should be updated to editor/game
   // VkDrawIndexedIndirectCommand newModel = {};
   // newModel.firstIndex = m_currentIndex;
   // newModel.indexCount = INDICES_PER_SPRITE;
   // newModel.firstInstance = 0;
   // newModel.instanceCount = 1;
   // newModel.vertexOffset = static_cast< int32_t >(m_currentVertex);
   //  m_renderCommands.push_back(newModel);

   // m_currentVertex += static_cast< uint32_t >(vertices_in.size());
   // m_currentIndex += INDICES_PER_SPRITE;

   PerInstanceBuffer newInstance = {};
   newInstance.model = modelMat;
   newInstance.color = color;

   for (const auto& texture : textures_in)
   {
      if (texture.empty())
      {
         continue;
      }

      const auto loadedTexture = TextureLibrary::GetTexture(texture);
      const auto idx = loadedTexture->GetID();

      switch (loadedTexture->GetType())
      {
         case TextureType::DIFFUSE_MAP: {
            newInstance.diffuse = idx;
         }
         break;
         default:
            break;
      }
   }

   renderData->perInstance.push_back(newInstance);

   UpdateDescriptors();

   return (*numObjects)++;
}

void
VulkanRenderer::SubmitMeshData(const uint32_t idx, const TextureID id, const glm::mat4& modelMat,
                               const glm::vec4& color)
{
   auto& object = Data::renderData_[boundApplication_].perInstance.at(idx);

   object.model = modelMat;
   object.color = color;
   object.diffuse = id;

   if (isLoaded_ and updateDescriptors_)
   {
      QuadShader::UpdateDescriptorSets();
   }
}

void
VulkanRenderer::CreateVertexBuffer()
{
   auto& renderData = Data::renderData_.at(boundApplication_);
   auto& vertices = renderData.vertices;

   const VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

   VkBuffer stagingBuffer = {};
   VkDeviceMemory stagingBufferMemory = {};
   Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        stagingBuffer, stagingBufferMemory);

   void* data = nullptr;
   vkMapMemory(Data::vk_device, stagingBufferMemory, 0, bufferSize, 0, &data);
   memcpy(data, vertices.data(), bufferSize);
   vkUnmapMemory(Data::vk_device, stagingBufferMemory);

   Buffer::CreateBuffer(
      bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, renderData.vertexBuffer, renderData.vertexBufferMemory);

   Buffer::CopyBuffer(stagingBuffer, renderData.vertexBuffer, bufferSize);

   vkDestroyBuffer(Data::vk_device, stagingBuffer, nullptr);
   vkFreeMemory(Data::vk_device, stagingBufferMemory, nullptr);
}

void
VulkanRenderer::CreateLinePipeline()
{
   LineShader::CreateDescriptorSetLayout();
   LineShader::CreateDescriptorPool();
   LineShader::CreateDescriptorSets();

   CreatePipeline< LineShader, LineVertex >("line.vert.spv", "line.frag.spv", PrimitiveType::LINE);
}

void
VulkanRenderer::DrawLine(const glm::vec2& start, const glm::vec2& end)
{
   Data::lineVertices_.push_back(LineVertex{glm::vec3{start, 0.0f}});
   Data::lineVertices_.push_back(LineVertex{glm::vec3{end, 0.0f}});

   ++Data::numLines;
}

void
VulkanRenderer::DrawDynamicLine(const glm::vec2& start, const glm::vec2& end)
{
   const auto totalNumVtx = Data::numGridLines * VERTICES_PER_LINE;
   if (Data::lineVertices_.size() < (totalNumVtx + Data::curDynLineIdx + VERTICES_PER_LINE))
   {
      Data::lineVertices_.push_back(LineVertex{glm::vec3{start, 0.0f}});
      Data::lineVertices_.push_back(LineVertex{glm::vec3{end, 0.0f}});
   }
   else
   {
      Data::lineVertices_[totalNumVtx + Data::curDynLineIdx++] = LineVertex{glm::vec3{start, 0.0f}};
      Data::lineVertices_[totalNumVtx + Data::curDynLineIdx++] = LineVertex{glm::vec3{end, 0.0f}};
   }
}

void
VulkanRenderer::UpdateLineData(uint32_t startingLine)
{
   const auto lastLine = (Data::curDynLineIdx / VERTICES_PER_LINE) + Data::numGridLines;
   const auto numLines = lastLine - startingLine;
   const auto bufferSize = numLines * sizeof(LineVertex) * VERTICES_PER_LINE;
   if (bufferSize)
   {
      const auto offset = startingLine * sizeof(LineVertex) * VERTICES_PER_LINE;

      void* data = nullptr;
      vkMapMemory(Data::vk_device, Data::lineVertexBufferMemory, offset, bufferSize, 0, &data);
      memcpy(data, Data::lineVertices_.data() + startingLine * VERTICES_PER_LINE, bufferSize);
      vkUnmapMemory(Data::vk_device, Data::lineVertexBufferMemory);
   }
}

void
VulkanRenderer::SetupLineData()
{
   {
      const VkDeviceSize bufferSize = sizeof(Vertex) * EditorData::MAX_NUM_LINES * 2;

      Buffer::CreateBuffer(
         bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
         Data::lineVertexBuffer, Data::lineVertexBufferMemory);
   }
   {
      auto& indices = Data::lineIndices_;
      indices.resize(static_cast< size_t >(EditorData::MAX_NUM_LINES)
                     * static_cast< size_t >(INDICES_PER_LINE));

      uint32_t offset = 0;
      for (uint32_t i = 0; i < indices.size(); i += INDICES_PER_LINE)
      {
         indices[i + 0] = offset + 0;
         indices[i + 1] = offset + 1;

         offset += 2;
      }

      const VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

      VkBuffer stagingBuffer = {};
      VkDeviceMemory stagingBufferMemory = {};
      Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                              | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           stagingBuffer, stagingBufferMemory);

      void* data = nullptr;
      vkMapMemory(Data::vk_device, stagingBufferMemory, 0, bufferSize, 0, &data);
      memcpy(data, indices.data(), bufferSize);
      vkUnmapMemory(Data::vk_device, stagingBufferMemory);

      Buffer::CreateBuffer(
         bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Data::lineIndexBuffer, Data::lineIndexBufferMemory);

      Buffer::CopyBuffer(stagingBuffer, Data::lineIndexBuffer, bufferSize);

      vkDestroyBuffer(Data::vk_device, stagingBuffer, nullptr);
      vkFreeMemory(Data::vk_device, stagingBufferMemory, nullptr);
   }
}

void
VulkanRenderer::SetupEditorData(ObjectType type)
{
   switch (type)
   {
      case ObjectType::ANIMATION_POINT: {
         {
            auto& vertices = EditorData::animationVertices_;

            if (vertices.empty())
            {
               return;
            }

            const VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

            VkBuffer stagingBuffer = {};
            VkDeviceMemory stagingBufferMemory = {};
            Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                                    | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                 stagingBuffer, stagingBufferMemory);

            void* data = nullptr;
            vkMapMemory(Data::vk_device, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, vertices.data(), bufferSize);
            vkUnmapMemory(Data::vk_device, stagingBufferMemory);

            Buffer::CreateBuffer(
               bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, EditorData::animationVertexBuffer,
               EditorData::animationVertexBufferMemory);

            Buffer::CopyBuffer(stagingBuffer, EditorData::animationVertexBuffer, bufferSize);

            vkDestroyBuffer(Data::vk_device, stagingBuffer, nullptr);
            vkFreeMemory(Data::vk_device, stagingBufferMemory, nullptr);
         }

         {
            auto& indices = EditorData::animationIndices_;
            indices.resize(static_cast< size_t >(EditorData::numPoints_)
                           * static_cast< size_t >(INDICES_PER_SPRITE));

            uint32_t offset = 0;
            for (uint32_t i = 0; i < indices.size(); i += INDICES_PER_SPRITE)
            {
               indices[i + 0] = offset + 0;
               indices[i + 1] = offset + 1;
               indices[i + 2] = offset + 2;

               indices[i + 3] = offset + 0;
               indices[i + 4] = offset + 2;
               indices[i + 5] = offset + 3;

               offset += 4;
            }

            const VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

            VkBuffer stagingBuffer = {};
            VkDeviceMemory stagingBufferMemory = {};
            Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                                    | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                 stagingBuffer, stagingBufferMemory);

            void* data = nullptr;
            vkMapMemory(Data::vk_device, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, indices.data(), bufferSize);
            vkUnmapMemory(Data::vk_device, stagingBufferMemory);

            Buffer::CreateBuffer(
               bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, EditorData::animationIndexBuffer,
               EditorData::animationIndexBufferMemory);

            Buffer::CopyBuffer(stagingBuffer, EditorData::animationIndexBuffer, bufferSize);

            vkDestroyBuffer(Data::vk_device, stagingBuffer, nullptr);
            vkFreeMemory(Data::vk_device, stagingBufferMemory, nullptr);
         }
      }
      break;
      case ObjectType::PATHFINDER_NODE: {
         {
            auto& vertices = EditorData::pathfinderVertices_;

            const VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

            VkBuffer stagingBuffer = {};
            VkDeviceMemory stagingBufferMemory = {};
            Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                                    | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                 stagingBuffer, stagingBufferMemory);

            void* data = nullptr;
            vkMapMemory(Data::vk_device, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, vertices.data(), bufferSize);
            vkUnmapMemory(Data::vk_device, stagingBufferMemory);

            Buffer::CreateBuffer(
               bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, EditorData::pathfinderVertexBuffer,
               EditorData::pathfinderVertexBufferMemory);

            Buffer::CopyBuffer(stagingBuffer, EditorData::pathfinderVertexBuffer, bufferSize);

            vkDestroyBuffer(Data::vk_device, stagingBuffer, nullptr);
            vkFreeMemory(Data::vk_device, stagingBufferMemory, nullptr);
         }

         {
            auto& indices = EditorData::pathfinderIndices_;
            indices.resize(static_cast< size_t >(EditorData::numNodes_)
                           * static_cast< size_t >(INDICES_PER_SPRITE));

            uint32_t offset = 0;
            for (uint32_t i = 0; i < indices.size(); i += INDICES_PER_SPRITE)
            {
               indices[i + 0] = offset + 0;
               indices[i + 1] = offset + 1;
               indices[i + 2] = offset + 2;

               indices[i + 3] = offset + 0;
               indices[i + 4] = offset + 2;
               indices[i + 5] = offset + 3;

               offset += 4;
            }

            const VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

            VkBuffer stagingBuffer = {};
            VkDeviceMemory stagingBufferMemory = {};
            Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                                    | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                 stagingBuffer, stagingBufferMemory);

            void* data = nullptr;
            vkMapMemory(Data::vk_device, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, indices.data(), bufferSize);
            vkUnmapMemory(Data::vk_device, stagingBufferMemory);

            Buffer::CreateBuffer(
               bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, EditorData::pathfinderIndexBuffer,
               EditorData::pathfinderIndexBufferMemory);

            Buffer::CopyBuffer(stagingBuffer, EditorData::pathfinderIndexBuffer, bufferSize);

            vkDestroyBuffer(Data::vk_device, stagingBuffer, nullptr);
            vkFreeMemory(Data::vk_device, stagingBufferMemory, nullptr);
         }
      }
      break;
      default: {
      }
   }

   UpdatePerInstanceBuffer();
}

void
VulkanRenderer::UpdateBuffers()
{
   vkDeviceWaitIdle(Data::vk_device);

   CreateVertexBuffer();
   CreateIndexBuffer();
   CreateUniformBuffer();
   CreatePerInstanceBuffer();
}

void
VulkanRenderer::UpdatePerInstanceBuffer()
{
   auto& renderData = Data::renderData_[boundApplication_];
   const VkDeviceSize SSBObufferSize = renderData.perInstance.size() * sizeof(PerInstanceBuffer);

   const auto swapchainImagesSize = MAX_FRAMES_IN_FLIGHT;

   renderData.ssbo.resize(swapchainImagesSize);
   renderData.ssboMemory.resize(swapchainImagesSize);

   for (size_t i = 0; i < swapchainImagesSize; i++)
   {
      Buffer::CreateBuffer(SSBObufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                              | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           renderData.ssbo[i], renderData.ssboMemory[i]);
   }
}

void
VulkanRenderer::SetupData(bool recreatePipeline)
{
   vkDeviceWaitIdle(Data::vk_device);

   auto& renderData = renderer::Data::renderData_.at(boundApplication_);
   vkDestroyDescriptorPool(Data::vk_device, renderData.descriptorPool, nullptr);

   UpdateBuffers();

   // Indirect render stuff
   // const auto commandsSize = m_renderCommands.size() * sizeof(VkDrawIndexedIndirectCommand);

   //////  Commands + draw count
   // const VkDeviceSize bufferSize = commandsSize + sizeof(uint32_t);

   // Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
   //                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
   //                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_indirectDrawsBuffer,
   //                      m_indirectDrawsBufferMemory);

   // void* data = nullptr;
   // vkMapMemory(Data::vk_device, m_indirectDrawsBufferMemory, 0, bufferSize, 0, &data);
   // memcpy(data, m_renderCommands.data(), static_cast< size_t >(bufferSize));
   // memcpy(static_cast< uint8_t* >(data) + commandsSize, &m_numMeshes, sizeof(uint32_t));

   if (recreatePipeline)
   {
      DestroyPipeline();

      CreateRenderPipeline();
      QuadShader::CreateDescriptorPool();
      QuadShader::CreateDescriptorSets();
      QuadShader::UpdateDescriptorSets();
   }


   isLoaded_ = true;
}

void
VulkanRenderer::DestroyPipeline()
{
   vkDeviceWaitIdle(Data::vk_device);

   auto& renderData = Data::renderData_.at(boundApplication_);

   for (size_t i = 0; i < renderData.swapChainImages.size(); ++i)
   {
      vkDestroyFramebuffer(Data::vk_device, renderData.swapChainFramebuffers[i], nullptr);
      vkDestroyImageView(Data::vk_device, renderData.swapChainImageViews[i], nullptr);
   }

   vkDestroySwapchainKHR(Data::vk_device, renderData.swapChain, nullptr);
   vkDestroySurfaceKHR(Data::vk_instance, renderData.surface, nullptr);

   renderData.swapChainImages.clear();
   renderData.swapChainImageViews.clear();
   renderData.swapChainFramebuffers.clear();
   renderData.descriptorSets.clear();
   renderData.swapChainImageFormat = VK_FORMAT_UNDEFINED;

   vkDestroyImage(Data::vk_device, renderData.colorImage, nullptr);
   vkDestroyImageView(Data::vk_device, renderData.colorImageView, nullptr);
   vkFreeMemory(Data::vk_device, renderData.colorImageMemory, nullptr);

   vkDestroyImage(Data::vk_device, renderData.depthImage, nullptr);
   vkDestroyImageView(Data::vk_device, renderData.depthImageView, nullptr);
   vkFreeMemory(Data::vk_device, renderData.depthImageMemory, nullptr);

   vkDestroyPipeline(Data::vk_device, renderData.pipeline, nullptr);
   vkDestroyPipelineLayout(Data::vk_device, renderData.pipelineLayout, nullptr);
   vkDestroyPipelineCache(Data::vk_device, renderData.pipelineCache, nullptr);
   vkDestroyRenderPass(Data::vk_device, renderData.renderPass, nullptr);
}

void
VulkanRenderer::FreeData(renderer::ApplicationType type)
{
   vkDeviceWaitIdle(Data::vk_device);

   if (Data::renderData_.find(type) != Data::renderData_.end())
   {
      auto& renderData = Data::renderData_.at(type);
      vkDestroyBuffer(Data::vk_device, renderData.indexBuffer, nullptr);
      vkFreeMemory(Data::vk_device, renderData.indexBufferMemory, nullptr);
      renderData.indices.clear();

      vkDestroyBuffer(Data::vk_device, renderData.vertexBuffer, nullptr);
      vkFreeMemory(Data::vk_device, renderData.vertexBufferMemory, nullptr);
      renderData.vertices.clear();

      for (size_t i = 0; i < renderData.uniformBuffers.size(); ++i)
      {
         vkDestroyBuffer(Data::vk_device, renderData.uniformBuffers[i], nullptr);
         vkFreeMemory(Data::vk_device, renderData.uniformBuffersMemory[i], nullptr);

         vkDestroyBuffer(Data::vk_device, renderData.ssbo[i], nullptr);
         vkFreeMemory(Data::vk_device, renderData.ssboMemory[i], nullptr);
      }

      renderData.perInstance.clear();
      renderData.ssbo.clear();
      renderData.ssboMemory.clear();

      DestroyPipeline();

      Data::renderData_.erase(type);
   }
}

void
VulkanRenderer::CreateIndexBuffer()
{
   auto& indices = Data::renderData_[boundApplication_].indices;
   indices.resize(static_cast< size_t >(Data::renderData_[boundApplication_].numMeshes)
                  * static_cast< size_t >(INDICES_PER_SPRITE));

   uint32_t offset = 0;
   for (uint32_t i = 0; i < indices.size(); i += INDICES_PER_SPRITE)
   {
      indices[i + 0] = offset + 0;
      indices[i + 1] = offset + 1;
      indices[i + 2] = offset + 2;

      indices[i + 3] = offset + 0;
      indices[i + 4] = offset + 2;
      indices[i + 5] = offset + 3;

      offset += 4;
   }

   const VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

   VkBuffer stagingBuffer = {};
   VkDeviceMemory stagingBufferMemory = {};
   Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        stagingBuffer, stagingBufferMemory);

   void* data = nullptr;
   vkMapMemory(Data::vk_device, stagingBufferMemory, 0, bufferSize, 0, &data);
   memcpy(data, indices.data(), bufferSize);
   vkUnmapMemory(Data::vk_device, stagingBufferMemory);

   Buffer::CreateBuffer(
      bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Data::renderData_[boundApplication_].indexBuffer,
      Data::renderData_[boundApplication_].indexBufferMemory);

   Buffer::CopyBuffer(stagingBuffer, Data::renderData_[boundApplication_].indexBuffer, bufferSize);

   vkDestroyBuffer(Data::vk_device, stagingBuffer, nullptr);
   vkFreeMemory(Data::vk_device, stagingBufferMemory, nullptr);
}

void
VulkanRenderer::CreateUniformBuffer()
{
   auto& renderData = Data::renderData_[boundApplication_];
   const VkDeviceSize bufferSize = sizeof(UniformBufferObject);

   // We always (for now) create buffers for all frames in flight, so we only have to check the
   // first one
   if (renderData.uniformBuffers[0] != VK_NULL_HANDLE)
   {
      for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
      {
         vkDestroyBuffer(Data::vk_device, renderData.uniformBuffers[i], nullptr);
         vkFreeMemory(Data::vk_device, renderData.uniformBuffersMemory[i], nullptr);
      }
   }

   for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
   {
      Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                              | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           renderData.uniformBuffers[i], renderData.uniformBuffersMemory[i]);
   }
}

void
VulkanRenderer::CreatePerInstanceBuffer()
{
   auto& renderData = Data::renderData_[boundApplication_];
   const VkDeviceSize SSBObufferSize = renderData.perInstance.size() * sizeof(PerInstanceBuffer);

   // We always (for now) create buffers for all frames in flight, so we only have to check the
   // first one
   if (renderData.ssboMemory[0] != VK_NULL_HANDLE)
   {
      for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
      {
         vkDestroyBuffer(Data::vk_device, renderData.ssbo[i], nullptr);
         vkFreeMemory(Data::vk_device, renderData.ssboMemory[i], nullptr);
      }
   }

   for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
   {
      Buffer::CreateBuffer(SSBObufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                              | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           renderData.ssbo[i], renderData.ssboMemory[i]);
   }
}

void
VulkanRenderer::Initialize(GLFWwindow* windowHandle, ApplicationType type)
{
   SetAppMarker(type);
   auto& renderData = Data::renderData_[boundApplication_];
   renderData.windowHandle = windowHandle;


   if (not initialized_)
   {
      CreateInstance();
   }

   vk_check_error(glfwCreateWindowSurface(Data::vk_instance, renderData.windowHandle, nullptr,
                                          &renderData.surface),
                  "failed to create window surface!");

   if (not initialized_)
   {
      CreateDevice();
   }

   CreateRenderPipeline();

   initialized_ = true;
}

void
VulkanRenderer::CreateRenderPipeline()
{
   auto& renderData = Data::renderData_[boundApplication_];

   vk_check_error(glfwCreateWindowSurface(Data::vk_instance, renderData.windowHandle, nullptr,
                                          &renderData.surface),
                  "failed to create window surface!");

   CreateSwapchain();
   CreateImageViews();
   CreateCommandPool();
   CreateRenderPass();
   QuadShader::CreateDescriptorSetLayout();
   CreatePipeline< QuadShader, Vertex >("vert.spv", "frag.spv", PrimitiveType::TRIANGLE);
   CreateColorResources();
   CreateDepthResources();
   CreateFramebuffers();
   CreatePipelineCache();
   CreateSyncObjects();
}

void
VulkanRenderer::UpdateUniformBuffer(uint32_t currentImage)
{
   auto& renderData = Data::renderData_.at(boundApplication_);
   if (renderData.uniformBuffersMemory[currentImage] != VK_NULL_HANDLE
       and renderData.ssboMemory[currentImage] != VK_NULL_HANDLE)
   {
      UniformBufferObject ubo = {};

      ubo.view = view_mat;
      ubo.proj = proj_mat;

      void* data = nullptr;
      vkMapMemory(Data::vk_device, renderData.uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0,
                  &data);
      memcpy(data, &ubo, sizeof(ubo));
      vkUnmapMemory(Data::vk_device, renderData.uniformBuffersMemory[currentImage]);

      void* data2 = nullptr;
      vkMapMemory(Data::vk_device, renderData.ssboMemory[currentImage], 0,
                  renderData.perInstance.size() * sizeof(PerInstanceBuffer), 0, &data2);
      memcpy(data2, renderData.perInstance.data(),
             renderData.perInstance.size() * sizeof(PerInstanceBuffer));
      vkUnmapMemory(Data::vk_device, renderData.ssboMemory[currentImage]);
   }
}

void
VulkanRenderer::CreateColorResources()
{
   auto& renderData = Data::renderData_.at(boundApplication_);

   std::tie(renderData.colorImage, renderData.colorImageMemory) = Texture::CreateImage(
      renderData.swapChainExtent.width, renderData.swapChainExtent.height, 1,
      renderer::Data::msaaSamples, renderData.swapChainImageFormat, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

   renderData.colorImageView = Texture::CreateImageView(
      renderData.colorImage, renderData.swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void
VulkanRenderer::CreateDepthResources()
{
   const auto depthFormat = FindDepthFormat();
   auto& renderData = Data::renderData_.at(boundApplication_);

   const auto [depthImage, depthImageMemory] = Texture::CreateImage(
      renderData.swapChainExtent.width, renderData.swapChainExtent.height, 1,
      renderer::Data::msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

   renderData.depthImage = depthImage;
   renderData.depthImageMemory = depthImageMemory;

   renderData.depthImageView =
      Texture::CreateImageView(renderData.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

VkFormat
VulkanRenderer::FindSupportedFormat(const std::vector< VkFormat >& candidates, VkImageTiling tiling,
                                    VkFormatFeatureFlags features)
{
   for (const auto format : candidates)
   {
      VkFormatProperties props = {};
      vkGetPhysicalDeviceFormatProperties(Data::vk_physicalDevice, format, &props);
      const auto tiling_linear =
         tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features;
      const auto tiling_optimal =
         tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features;

      if (tiling_linear or tiling_optimal)
      {
         return format;
      }
   }

   utils::Assert(false, "failed to find supported format!");
   return {};
}

VkFormat
VulkanRenderer::FindDepthFormat()
{
   return FindSupportedFormat({VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                              VK_IMAGE_TILING_OPTIMAL,
                              VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void
VulkanRenderer::Render(Application* app)
{
   vkWaitForFences(Data::vk_device, 1, &m_inFlightFences[Data::currentFrame_], VK_TRUE, UINT64_MAX);

   uint32_t imageIndex = {};
   vkAcquireNextImageKHR(Data::vk_device, Data::renderData_[boundApplication_].swapChain,
                         UINT64_MAX, m_imageAvailableSemaphores[Data::currentFrame_],
                         VK_NULL_HANDLE, &imageIndex);

   CreateCommandBuffers(app, imageIndex);
   UpdateUniformBuffer(Data::currentFrame_);

   VkSubmitInfo submitInfo = {};
   submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

   auto waitStages =
      std::to_array< const VkPipelineStageFlags >({VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT});
   submitInfo.waitSemaphoreCount = 1;
   submitInfo.pWaitSemaphores = &m_imageAvailableSemaphores[Data::currentFrame_];
   submitInfo.pWaitDstStageMask = waitStages.data();

   submitInfo.commandBufferCount = 1;
   submitInfo.pCommandBuffers = &Data::commandBuffers[Data::currentFrame_];

   submitInfo.signalSemaphoreCount = 1;
   submitInfo.pSignalSemaphores = &m_renderFinishedSemaphores[Data::currentFrame_];

   vkResetFences(Data::vk_device, 1, &m_inFlightFences[Data::currentFrame_]);

   vk_check_error(
      vkQueueSubmit(Data::vk_graphicsQueue, 1, &submitInfo, m_inFlightFences[Data::currentFrame_]),
      "failed to submit draw command buffer!");

   VkPresentInfoKHR presentInfo = {};
   presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

   presentInfo.waitSemaphoreCount = 1;
   presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[Data::currentFrame_];

   presentInfo.swapchainCount = 1;
   presentInfo.pSwapchains = &Data::renderData_[boundApplication_].swapChain;

   presentInfo.pImageIndices = &imageIndex;

   vkQueuePresentKHR(m_presentQueue, &presentInfo);

   Data::currentFrame_ = (Data::currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;

   updateDescriptors_ = false;
}

void
VulkanRenderer::CreateInstance()
{
   if (ENABLE_VALIDATION && !checkValidationLayerSupport())
   {
      utils::Assert(false, "validation layers asked but not supported!");
   }

   VkApplicationInfo appInfo = {};
   appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
   appInfo.pApplicationName = "looper";
   appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
   appInfo.apiVersion = VK_API_VERSION_1_3;

   VkInstanceCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   createInfo.pApplicationInfo = &appInfo;

   const auto extensions = getRequiredExtensions();
   createInfo.enabledExtensionCount = static_cast< uint32_t >(extensions.size());
   createInfo.ppEnabledExtensionNames = extensions.data();

   if constexpr (ENABLE_VALIDATION)
   {
      createInfo.enabledLayerCount = static_cast< uint32_t >(VALIDATION_LAYERS.size());
      createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

      populateDebugMessengerCreateInfo(m_debugCreateInfo);
      createInfo.pNext = static_cast< void* >(&m_debugCreateInfo);
   }

   vk_check_error(vkCreateInstance(&createInfo, nullptr, &Data::vk_instance),
                  "instance not created properly!");
}

void
VulkanRenderer::CreateDevice()
{
   uint32_t deviceCount = 0;
   vkEnumeratePhysicalDevices(Data::vk_instance, &deviceCount, nullptr);
   utils::Assert(deviceCount > 0, "failed to find GPUs with Vulkan support!");

   std::vector< VkPhysicalDevice > devices(deviceCount);
   vkEnumeratePhysicalDevices(Data::vk_instance, &deviceCount, devices.data());

   const auto device = std::find_if(devices.begin(), devices.end(), [](const auto& device) {
      return isDeviceSuitable(device, Data::renderData_[boundApplication_].surface);
   });

   utils::Assert(device != devices.end(), "Unable to find suitable device!");

   VkPhysicalDeviceProperties deviceProps = {};
   vkGetPhysicalDeviceProperties(*device, &deviceProps);
   Logger::Info("Device found! Using {}", deviceProps.deviceName);

   Data::vk_physicalDevice = *device;
   renderer::Data::msaaSamples = getMaxUsableSampleCount(Data::vk_physicalDevice);

   utils::Assert(Data::vk_physicalDevice != VK_NULL_HANDLE, "failed to find a suitable GPU!");

   const auto indices_ =
      findQueueFamilies(Data::vk_physicalDevice, Data::renderData_[boundApplication_].surface);

   std::vector< VkDeviceQueueCreateInfo > queueCreateInfos = {};
   const std::set< uint32_t > uniqueQueueFamilies = {indices_.GetGraphics(), indices_.GetPresent()};

   constexpr float queuePriority = 1.0f;
   for (const auto queueFamily : uniqueQueueFamilies)
   {
      VkDeviceQueueCreateInfo queueCreateInfo = {};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;

      queueCreateInfos.push_back(queueCreateInfo);
   }

   VkPhysicalDeviceVulkan12Features deviceFeatures_12 = {};
   deviceFeatures_12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
   deviceFeatures_12.drawIndirectCount = VK_TRUE;

   VkPhysicalDeviceVulkan11Features deviceFeatures_11 = {};
   deviceFeatures_11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
   deviceFeatures_11.pNext = &deviceFeatures_12;
   deviceFeatures_11.shaderDrawParameters = VK_TRUE;

   VkPhysicalDeviceFeatures deviceFeatures = {};
   deviceFeatures.samplerAnisotropy = VK_TRUE;
   deviceFeatures.multiDrawIndirect = VK_TRUE;

   VkDeviceCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
   createInfo.pNext = &deviceFeatures_11;

   createInfo.queueCreateInfoCount = static_cast< uint32_t >(queueCreateInfos.size());
   createInfo.pQueueCreateInfos = queueCreateInfos.data();

   createInfo.pEnabledFeatures = &deviceFeatures;

   createInfo.enabledExtensionCount = static_cast< uint32_t >(DEVICE_EXTENSIONS.size());
   createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

   if constexpr (ENABLE_VALIDATION)
   {
      createInfo.enabledLayerCount = static_cast< uint32_t >(VALIDATION_LAYERS.size());
      createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
   }

   vk_check_error(vkCreateDevice(Data::vk_physicalDevice, &createInfo, nullptr, &Data::vk_device),
                  "failed to create logical device!");

   vkGetDeviceQueue(Data::vk_device, indices_.graphicsFamily.value(), 0, &Data::vk_graphicsQueue);
   vkGetDeviceQueue(Data::vk_device, indices_.presentFamily.value(), 0, &m_presentQueue);
}

void
VulkanRenderer::CreateSwapchain()
{
   auto& renderData = Data::renderData_.at(boundApplication_);

   const auto swapChainSupport = querySwapChainSupport(Data::vk_physicalDevice, renderData.surface);
   const auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
   const auto presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
   const auto extent = chooseSwapExtent(swapChainSupport.capabilities, renderData.windowHandle);

   uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
   if (swapChainSupport.capabilities.maxImageCount > 0
       && imageCount > swapChainSupport.capabilities.maxImageCount)
   {
      imageCount = swapChainSupport.capabilities.maxImageCount;
   }

   VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
   swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
   swapChainCreateInfo.surface = renderData.surface;

   swapChainCreateInfo.minImageCount = imageCount;
   swapChainCreateInfo.imageFormat = surfaceFormat.format;
   swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
   swapChainCreateInfo.imageExtent = extent;
   swapChainCreateInfo.imageArrayLayers = 1;
   swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

   const auto indicesSecond = findQueueFamilies(Data::vk_physicalDevice, renderData.surface);
   const auto queueFamilyIndices =
      std::to_array({indicesSecond.GetGraphics(), indicesSecond.GetPresent()});

   if (indicesSecond.graphicsFamily != indicesSecond.presentFamily)
   {
      swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      swapChainCreateInfo.queueFamilyIndexCount = 2;
      swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
   }
   else
   {
      swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
   }

   swapChainCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
   swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
   swapChainCreateInfo.presentMode = presentMode;
   swapChainCreateInfo.clipped = VK_TRUE;

   vk_check_error(
      vkCreateSwapchainKHR(Data::vk_device, &swapChainCreateInfo, nullptr, &renderData.swapChain),
      "failed to create swap chain!");

   vkGetSwapchainImagesKHR(Data::vk_device, renderData.swapChain, &imageCount, nullptr);
   renderData.swapChainImages.resize(imageCount);
   vkGetSwapchainImagesKHR(Data::vk_device, renderData.swapChain, &imageCount,
                           renderData.swapChainImages.data());

   renderData.swapChainImageFormat = surfaceFormat.format;
   renderData.swapChainExtent = extent;
}

void
VulkanRenderer::CreateImageViews()
{
   auto& renderData = Data::renderData_.at(boundApplication_);

   renderData.swapChainImageViews.resize(renderData.swapChainImages.size());

   for (uint32_t i = 0; i < renderData.swapChainImages.size(); i++)
   {
      renderData.swapChainImageViews[i] =
         Texture::CreateImageView(renderData.swapChainImages[i], renderData.swapChainImageFormat,
                                  VK_IMAGE_ASPECT_COLOR_BIT, 1);
   }
}

void
VulkanRenderer::CreateRenderPass()
{
   auto& renderData = Data::renderData_.at(boundApplication_);

   VkAttachmentDescription colorAttachment = {};
   colorAttachment.format = renderData.swapChainImageFormat;
   // colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
   colorAttachment.samples = renderer::Data::msaaSamples;
   colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
   colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
   colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
   colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
   colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

   VkAttachmentDescription depthStencilAttachment = {};
   depthStencilAttachment.format = FindDepthFormat();
   depthStencilAttachment.samples = renderer::Data::msaaSamples;
   depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
   depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
   depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   // Load stencil buffer
   depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE; // Store stencil buffer
   depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   depthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

   VkAttachmentDescription colorAttachmentResolve = {};
   colorAttachmentResolve.format = renderData.swapChainImageFormat;
   colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
   colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
   colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
   colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
   colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
   colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

   VkAttachmentReference colorAttachmentRef = {};
   colorAttachmentRef.attachment = 0;
   colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

   VkAttachmentReference depthStencilAttachmentRef = {};
   depthStencilAttachmentRef.attachment = 1;
   depthStencilAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

   VkAttachmentReference colorAttachmentResolveRef = {};
   colorAttachmentResolveRef.attachment = 2;
   colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

   VkSubpassDescription subpass = {};
   subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
   subpass.colorAttachmentCount = 1;
   subpass.pColorAttachments = &colorAttachmentRef;
   subpass.pDepthStencilAttachment = &depthStencilAttachmentRef;
   subpass.pResolveAttachments = &colorAttachmentResolveRef;

   VkSubpassDependency dependency = {};
   dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
   dependency.dstSubpass = 0;
   dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   dependency.srcAccessMask = 0;
   dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

   std::array< VkAttachmentDescription, 3 > attachments = {colorAttachment, depthStencilAttachment,
                                                           colorAttachmentResolve};
   VkRenderPassCreateInfo renderPassInfo = {};
   renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
   renderPassInfo.attachmentCount = static_cast< uint32_t >(attachments.size());
   renderPassInfo.pAttachments = attachments.data();
   renderPassInfo.subpassCount = 1;
   renderPassInfo.pSubpasses = &subpass;
   renderPassInfo.dependencyCount = 1;
   renderPassInfo.pDependencies = &dependency;

   vk_check_error(
      vkCreateRenderPass(Data::vk_device, &renderPassInfo, nullptr, &renderData.renderPass),
      "failed to create render pass!");
}

void
VulkanRenderer::CreateFramebuffers()
{
   auto& renderData = Data::renderData_.at(boundApplication_);

   renderData.swapChainFramebuffers.resize(renderData.swapChainImageViews.size());

   for (size_t i = 0; i < renderData.swapChainImageViews.size(); i++)
   {
      std::array< VkImageView, 3 > attachments = {
         renderData.colorImageView, renderData.depthImageView, renderData.swapChainImageViews[i]};

      VkFramebufferCreateInfo framebufferInfo = {};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = renderData.renderPass;
      framebufferInfo.attachmentCount = static_cast< uint32_t >(attachments.size());
      framebufferInfo.pAttachments = attachments.data();
      framebufferInfo.width = renderData.swapChainExtent.width;
      framebufferInfo.height = renderData.swapChainExtent.height;
      framebufferInfo.layers = 1;

      vk_check_error(vkCreateFramebuffer(Data::vk_device, &framebufferInfo, nullptr,
                                         &renderData.swapChainFramebuffers[i]),
                     "Failed to create framebuffer!");
   }
}

void
VulkanRenderer::CreateCommandPool()
{
   auto queueFamilyIndicesTwo =
      findQueueFamilies(Data::vk_physicalDevice, Data::renderData_[boundApplication_].surface);

   VkCommandPoolCreateInfo poolInfo = {};
   poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
   poolInfo.queueFamilyIndex = queueFamilyIndicesTwo.GetGraphics();

   vk_check_error(vkCreateCommandPool(Data::vk_device, &poolInfo, nullptr, &Data::commandPool),
                  "Failed to create command pool!");
}

void
VulkanRenderer::CreateCommandBuffers(Application* app, uint32_t imageIndex)
{
   auto& renderData = Data::renderData_.at(boundApplication_);

   if (Data::commandBuffers.empty())
   {
      Data::commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

      VkCommandBufferAllocateInfo allocInfo{};
      allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandPool = Data::commandPool;
      allocInfo.commandBufferCount = static_cast< uint32_t >(Data::commandBuffers.size());

      vk_check_error(
         vkAllocateCommandBuffers(Data::vk_device, &allocInfo, Data::commandBuffers.data()),
         "failed to allocate command buffers!");
   }

   VkCommandBufferBeginInfo beginInfo = {};
   beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

   std::array< VkClearValue, 2 > clearValues{};
   clearValues[0].color = {{0.3f, 0.5f, 0.1f, 1.0f}};
   clearValues[1].depthStencil = {1.0f, 0};

   VkRenderPassBeginInfo renderPassInfo{};
   renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   renderPassInfo.renderPass = renderData.renderPass;
   renderPassInfo.renderArea.offset = {0, 0};
   renderPassInfo.renderArea.extent = renderData.swapChainExtent;
   renderPassInfo.clearValueCount = static_cast< uint32_t >(clearValues.size());
   renderPassInfo.pClearValues = clearValues.data();

   renderPassInfo.framebuffer = renderData.swapChainFramebuffers[imageIndex];

   vk_check_error(vkBeginCommandBuffer(Data::commandBuffers[Data::currentFrame_], &beginInfo), "");

   vkCmdBeginRenderPass(Data::commandBuffers[Data::currentFrame_], &renderPassInfo,
                        VK_SUBPASS_CONTENTS_INLINE);

   VkViewport viewport = {};
   viewport.width = static_cast< float >(renderData.swapChainExtent.width);
   viewport.height = static_cast< float >(renderData.swapChainExtent.height);
   viewport.minDepth = 0.0f;
   viewport.maxDepth = 1.0f;

   vkCmdSetViewport(Data::commandBuffers[Data::currentFrame_], 0, 1, &viewport);

   VkRect2D scissor = {};
   scissor.extent.width = renderData.swapChainExtent.width;
   scissor.extent.height = renderData.swapChainExtent.height;
   scissor.offset.x = 0;
   scissor.offset.y = 0;

   vkCmdSetScissor(Data::commandBuffers[Data::currentFrame_], 0, 1, &scissor);

   app->Render(Data::commandBuffers[Data::currentFrame_]);

   vkCmdEndRenderPass(Data::commandBuffers[Data::currentFrame_]);
   vk_check_error(vkEndCommandBuffer(Data::commandBuffers[Data::currentFrame_]), "");
}

void
VulkanRenderer::CreateSyncObjects()
{
   auto& renderData = Data::renderData_.at(boundApplication_);

   m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
   m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
   m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
   m_imagesInFlight.resize(renderData.swapChainImages.size(), VK_NULL_HANDLE);

   VkSemaphoreCreateInfo semaphoreInfo = {};
   semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

   VkFenceCreateInfo fenceInfo = {};
   fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
   fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

   for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
   {
      vk_check_error(vkCreateSemaphore(Data::vk_device, &semaphoreInfo, nullptr,
                                       &m_imageAvailableSemaphores[i]),
                     fmt::format("Failed to create m_imageAvailableSemaphores[{}]!", i));
      vk_check_error(vkCreateSemaphore(Data::vk_device, &semaphoreInfo, nullptr,
                                       &m_renderFinishedSemaphores[i]),
                     fmt::format("Failed to create m_renderFinishedSemaphores[{}]!", i));
      vk_check_error(vkCreateFence(Data::vk_device, &fenceInfo, nullptr, &m_inFlightFences[i]),
                     fmt::format("Failed to create m_inFlightFences[{}]!", i));
   }
}

void
VulkanRenderer::CreatePipelineCache()
{
   VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
   pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
   vk_check_error(vkCreatePipelineCache(Data::vk_device, &pipelineCacheCreateInfo, nullptr,
                                        &Data::renderData_.at(boundApplication_).pipelineCache),
                  "");
}

} // namespace looper::renderer
