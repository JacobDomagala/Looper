
#include "renderer.hpp"
#include "application.hpp"
#include "buffer.hpp"
#include "command.hpp"
#include "helpers.hpp"
#include "logger/logger.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "utils/assert.hpp"
#include "utils/file_manager.hpp"
#include "vulkan_common.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <array>
#include <optional>

namespace looper::renderer {

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
      pushConstantRange.size = sizeof(typename ShaderType::PushConstants);

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

template < typename DataT >
void
CreateVertexBuffer(size_t bufferSize, const std::vector< DataT >& vertices, VkBuffer& buffer,
                   VkDeviceMemory& bufferMem)
{
   if (vertices.empty())
   {
      return;
   }

   VkBuffer stagingBuffer = {};
   VkDeviceMemory stagingBufferMemory = {};
   Buffer::CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        stagingBuffer, stagingBufferMemory);

   void* data = nullptr;
   vkMapMemory(Data::vk_device, stagingBufferMemory, 0, bufferSize, 0, &data);
   memcpy(data, vertices.data(), bufferSize);
   vkUnmapMemory(Data::vk_device, stagingBufferMemory);

   Buffer::CreateBuffer(bufferSize,
                        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMem);

   Buffer::CopyBuffer(stagingBuffer, buffer, bufferSize);

   vkDestroyBuffer(Data::vk_device, stagingBuffer, nullptr);
   vkFreeMemory(Data::vk_device, stagingBufferMemory, nullptr);
}

template < auto INDICES_PER_OBJECT >
void
CreateIndexBuffer(std::vector< IndexType >& indices, const size_t numObjects, VkBuffer& buffer,
                  VkDeviceMemory& bufferMem)
{
   indices.resize(numObjects * static_cast< size_t >(INDICES_PER_OBJECT));

   IndexType offset = 0;
   for (uint32_t i = 0; i < indices.size(); i += INDICES_PER_OBJECT)
   {
      if constexpr (INDICES_PER_OBJECT == INDICES_PER_SPRITE)
      {
         indices[i + 0] = offset + 0;
         indices[i + 1] = offset + 1;
         indices[i + 2] = offset + 2;

         indices[i + 3] = offset + 0;
         indices[i + 4] = offset + 2;
         indices[i + 5] = offset + 3;

         offset += 4;
      }
      else
      {
         indices[i + 0] = offset + 0;
         indices[i + 1] = offset + 1;

         offset += 2;
      }
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

   Buffer::CreateBuffer(bufferSize,
                        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMem);

   Buffer::CopyBuffer(stagingBuffer, buffer, bufferSize);

   vkDestroyBuffer(Data::vk_device, stagingBuffer, nullptr);
   vkFreeMemory(Data::vk_device, stagingBufferMemory, nullptr);
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

   std::ranges::copy(vertices_in, std::back_inserter(*vertices));
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

   // Leaving this in case we need Normal/Specular maps in future
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
VulkanRenderer::CreateQuadVertexBuffer()
{
   auto& renderData = Data::renderData_.at(boundApplication_);
   auto& vertices = renderData.vertices;

   const VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();
   CreateVertexBuffer(bufferSize, renderData.vertices, renderData.vertexBuffer,
                      renderData.vertexBufferMemory);
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
   Buffer::CreateBuffer(sizeof(LineVertex) * MAX_NUM_LINES * 2,
                        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        Data::lineVertexBuffer, Data::lineVertexBufferMemory);

   CreateIndexBuffer< INDICES_PER_LINE >(Data::lineIndices_, MAX_NUM_LINES,
                                         Data::lineIndexBuffer, Data::lineIndexBufferMemory);
}

void
VulkanRenderer::SetupEditorData(ObjectType type)
{
   switch (type)
   {
      case ObjectType::ANIMATION_POINT: {
         auto& vertices = EditorData::animationVertices_;

         CreateVertexBuffer(sizeof(Vertex) * vertices.size(), vertices,
                            EditorData::animationVertexBuffer,
                            EditorData::animationVertexBufferMemory);

         CreateIndexBuffer< INDICES_PER_SPRITE >(
            EditorData::animationIndices_, EditorData::numPoints_, EditorData::animationIndexBuffer,
            EditorData::animationIndexBufferMemory);
      }
      break;
      case ObjectType::PATHFINDER_NODE: {
         auto& vertices = EditorData::pathfinderVertices_;

         CreateVertexBuffer(sizeof(Vertex) * vertices.size(), vertices,
                            EditorData::pathfinderVertexBuffer,
                            EditorData::pathfinderVertexBufferMemory);

         CreateIndexBuffer< INDICES_PER_SPRITE >(
            EditorData::pathfinderIndices_, EditorData::numNodes_,
            EditorData::pathfinderIndexBuffer, EditorData::pathfinderIndexBufferMemory);
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

   CreateQuadVertexBuffer();
   CreateQuadIndexBuffer();
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
VulkanRenderer::CreateQuadIndexBuffer()
{
   auto& renderData = Data::renderData_.at(boundApplication_);

   CreateIndexBuffer< INDICES_PER_SPRITE >(renderData.indices, renderData.numMeshes,
                                           renderData.indexBuffer, renderData.indexBufferMemory);
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

      ubo.view = renderData.viewMat;
      ubo.proj = renderData.projMat;

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
   auto& renderData = Data::renderData_.at(boundApplication_);

   vkWaitForFences(Data::vk_device, 1, &inFlightFences_[Data::currentFrame_], VK_TRUE, UINT64_MAX);

   uint32_t imageIndex = {};
   vkAcquireNextImageKHR(Data::vk_device, renderData.swapChain,
                         UINT64_MAX, imageAvailableSemaphores_[Data::currentFrame_],
                         VK_NULL_HANDLE, &imageIndex);

   CreateCommandBuffers(app, imageIndex);
   UpdateUniformBuffer(Data::currentFrame_);

   VkSubmitInfo submitInfo = {};
   submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

   auto waitStages =
      std::to_array< const VkPipelineStageFlags >({VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT});
   submitInfo.waitSemaphoreCount = 1;
   submitInfo.pWaitSemaphores = &imageAvailableSemaphores_[Data::currentFrame_];
   submitInfo.pWaitDstStageMask = waitStages.data();

   submitInfo.commandBufferCount = 1;
   submitInfo.pCommandBuffers = &Data::commandBuffers[Data::currentFrame_];

   submitInfo.signalSemaphoreCount = 1;
   submitInfo.pSignalSemaphores = &renderFinishedSemaphores_[Data::currentFrame_];

   vkResetFences(Data::vk_device, 1, &inFlightFences_[Data::currentFrame_]);

   vk_check_error(
      vkQueueSubmit(Data::vk_graphicsQueue, 1, &submitInfo, inFlightFences_[Data::currentFrame_]),
      "failed to submit draw command buffer!");

   VkPresentInfoKHR presentInfo = {};
   presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

   presentInfo.waitSemaphoreCount = 1;
   presentInfo.pWaitSemaphores = &renderFinishedSemaphores_[Data::currentFrame_];

   presentInfo.swapchainCount = 1;
   presentInfo.pSwapchains = &renderData.swapChain;

   presentInfo.pImageIndices = &imageIndex;

   vkQueuePresentKHR(presentQueue_, &presentInfo);

   Data::currentFrame_ = (Data::currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;

   updateDescriptors_ = false;
}

void
VulkanRenderer::CreateInstance()
{
   if (ENABLE_VALIDATION && !CheckValidationLayerSupport())
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

   const auto extensions = GetRequiredExtensions();
   createInfo.enabledExtensionCount = static_cast< uint32_t >(extensions.size());
   createInfo.ppEnabledExtensionNames = extensions.data();

   if constexpr (ENABLE_VALIDATION)
   {
      createInfo.enabledLayerCount = static_cast< uint32_t >(VALIDATION_LAYERS.size());
      createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

      PopulateDebugMessengerCreateInfo(debugCreateInfo_);
      createInfo.pNext = static_cast< void* >(&debugCreateInfo_);
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
      return IsDeviceSuitable(device, Data::renderData_[boundApplication_].surface);
   });

   utils::Assert(device != devices.end(), "Unable to find suitable device!");

   VkPhysicalDeviceProperties deviceProps = {};
   vkGetPhysicalDeviceProperties(*device, &deviceProps);
   Logger::Info("Device found! Using {}", deviceProps.deviceName);

   Data::vk_physicalDevice = *device;
   renderer::Data::msaaSamples = GetMaxUsableSampleCount(Data::vk_physicalDevice);

   utils::Assert(Data::vk_physicalDevice != VK_NULL_HANDLE, "failed to find a suitable GPU!");

   const auto indices_ =
      FindQueueFamilies(Data::vk_physicalDevice, Data::renderData_[boundApplication_].surface);

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
   vkGetDeviceQueue(Data::vk_device, indices_.presentFamily.value(), 0, &presentQueue_);
}

void
VulkanRenderer::CreateSwapchain()
{
   auto& renderData = Data::renderData_.at(boundApplication_);

   const auto swapChainSupport = QuerySwapChainSupport(Data::vk_physicalDevice, renderData.surface);
   const auto surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
   const auto presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
   const auto extent = ChooseSwapExtent(swapChainSupport.capabilities, renderData.windowHandle);

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

   const auto indicesSecond = FindQueueFamilies(Data::vk_physicalDevice, renderData.surface);
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
      FindQueueFamilies(Data::vk_physicalDevice, Data::renderData_[boundApplication_].surface);

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
   imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
   renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
   inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);

   VkSemaphoreCreateInfo semaphoreInfo = {};
   semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

   VkFenceCreateInfo fenceInfo = {};
   fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
   fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

   for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
   {
      vk_check_error(vkCreateSemaphore(Data::vk_device, &semaphoreInfo, nullptr,
                                       &imageAvailableSemaphores_[i]),
                     fmt::format("Failed to create m_imageAvailableSemaphores[{}]!", i));
      vk_check_error(vkCreateSemaphore(Data::vk_device, &semaphoreInfo, nullptr,
                                       &renderFinishedSemaphores_[i]),
                     fmt::format("Failed to create m_renderFinishedSemaphores[{}]!", i));
      vk_check_error(vkCreateFence(Data::vk_device, &fenceInfo, nullptr, &inFlightFences_[i]),
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
