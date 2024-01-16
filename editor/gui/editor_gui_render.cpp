#include "animatable.hpp"
#include "editor.hpp"
#include "editor_gui.hpp"
#include "game_object.hpp"
#include "helpers.hpp"
#include "icons.hpp"
#include "renderer/renderer.hpp"
#include "renderer/shader.hpp"
#include "renderer/texture.hpp"
#include "renderer/types.hpp"
#include "renderer/vulkan_common.hpp"
#include "time/scoped_timer.hpp"
#include "types.hpp"
#include "utils/file_manager.hpp"

#include <GLFW/glfw3.h>
#include <fmt/format.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <cstdint>


namespace looper {

void
EditorGUI::UpdateBuffers()
{
   ImDrawData* imDrawData = ImGui::GetDrawData();

   // Update buffers only if vertex or index count has been changed compared to current buffer size
   if (!imDrawData or !imDrawData->TotalVtxCount or !imDrawData->TotalIdxCount)
   {
      return;
   }

   // Note: Alignment is done inside buffer creation
   const VkDeviceSize vertexBufferSize =
      static_cast< uint32_t >(imDrawData->TotalVtxCount) * sizeof(ImDrawVert);
   const VkDeviceSize indexBufferSize =
      static_cast< uint32_t >(imDrawData->TotalIdxCount) * sizeof(ImDrawIdx);

   const auto currentFrame = renderer::Data::currentFrame_;

   // Vertex buffer
   auto& vertexBuffer = renderer::EditorData::vertexBuffer_[currentFrame];
   auto& vertexCount = renderer::EditorData::vertexCount_[currentFrame];
   if ((vertexBuffer.buffer_ == VK_NULL_HANDLE) || (vertexCount < imDrawData->TotalVtxCount))
   {
      if (vertexBuffer.buffer_ != VK_NULL_HANDLE)
      {
         vertexBuffer.Unmap();
         vertexBuffer.Destroy();
      }

      vertexBuffer = renderer::Buffer::CreateBuffer(
         vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      vertexCount = imDrawData->TotalVtxCount;
      vertexBuffer.Map();
   }

   // Index buffer
   auto& indexBuffer = renderer::EditorData::indexBuffer_[currentFrame];
   auto& indexCount = renderer::EditorData::indexCount_[currentFrame];
   if ((indexBuffer.buffer_ == VK_NULL_HANDLE) || (indexCount < imDrawData->TotalIdxCount))
   {
      if (indexBuffer.buffer_ != VK_NULL_HANDLE)
      {
         indexBuffer.Unmap();
         indexBuffer.Destroy();
      }

      indexBuffer = renderer::Buffer::CreateBuffer(
         indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      indexCount = imDrawData->TotalIdxCount;
      indexBuffer.Map();
   }

   // Upload data
   auto* vtxDst = static_cast< ImDrawVert* >(vertexBuffer.mappedMemory_);
   auto* idxDst = static_cast< ImDrawIdx* >(indexBuffer.mappedMemory_);

   for (int n = 0; n < imDrawData->CmdListsCount; n++)
   {
      const ImDrawList* cmd_list = imDrawData->CmdLists[n];
      memcpy(vtxDst, cmd_list->VtxBuffer.Data,
             static_cast< size_t >(cmd_list->VtxBuffer.Size) * sizeof(ImDrawVert));
      memcpy(idxDst, cmd_list->IdxBuffer.Data,
             static_cast< size_t >(cmd_list->IdxBuffer.Size) * sizeof(ImDrawIdx));
      vtxDst += cmd_list->VtxBuffer.Size;
      idxDst += cmd_list->IdxBuffer.Size;
   }

   // Flush to make writes visible to GPU
   vertexBuffer.Flush();
   indexBuffer.Flush();
}

void
EditorGUI::Render(VkCommandBuffer commandBuffer)
{
   const time::ScopedTimer guiRender(&uiRenderTime);

   ImDrawData* imDrawData = ImGui::GetDrawData();
   int32_t vertexOffset = 0;
   uint32_t indexOffset = 0;

   if (!imDrawData or !imDrawData->CmdListsCount)
   {
      return;
   }

   UpdateBuffers();

   const ImGuiIO& io = ImGui::GetIO();
   const auto currentFrame = renderer::Data::currentFrame_;

   vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                     renderer::EditorData::pipeline_);

   renderer::EditorData::pushConstant_.scale =
      glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
   renderer::EditorData::pushConstant_.translate = glm::vec2(-1.0f);
   vkCmdPushConstants(commandBuffer, renderer::EditorData::pipelineLayout_,
                      VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(renderer::PushConstBlock),
                      &renderer::EditorData::pushConstant_);

   std::array< VkDeviceSize, 1 > offsets = {0};
   vkCmdBindVertexBuffers(commandBuffer, 0, 1,
                          &renderer::EditorData::vertexBuffer_[currentFrame].buffer_,
                          offsets.data());
   vkCmdBindIndexBuffer(commandBuffer, renderer::EditorData::indexBuffer_[currentFrame].buffer_, 0,
                        VK_INDEX_TYPE_UINT16);

   for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
   {
      const ImDrawList* cmd_list = imDrawData->CmdLists[i];
      for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
      {
         const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
         VkRect2D scissorRect;
         scissorRect.offset.x = static_cast< int32_t >(glm::max(pcmd->ClipRect.x, 0.0f));
         scissorRect.offset.y = static_cast< int32_t >(glm::max(pcmd->ClipRect.y, 0.0f));
         scissorRect.extent.width = static_cast< uint32_t >(pcmd->ClipRect.z - pcmd->ClipRect.x);
         scissorRect.extent.height = static_cast< uint32_t >(pcmd->ClipRect.w - pcmd->ClipRect.y);
         vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
         if (static_cast< VkDescriptorSet >(pcmd->TextureId) != VK_NULL_HANDLE)
         {
            const auto desc_set = std::to_array({static_cast< VkDescriptorSet >(pcmd->TextureId)});
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    renderer::EditorData::pipelineLayout_, 0, 1, desc_set.data(), 0,
                                    nullptr);
         }
         else
         {
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    renderer::EditorData::pipelineLayout_, 0, 1,
                                    &renderer::EditorData::descriptorSet_, 0, nullptr);
         }

         vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
         indexOffset += pcmd->ElemCount;
      }
      vertexOffset += cmd_list->VtxBuffer.Size;
   }
}

void
EditorGUI::PrepareResources()
{
   auto& io = ImGui::GetIO();

   const auto fontFilename = (FONTS_DIR / "Roboto-Medium.ttf").string();

   constexpr auto baseFontSize = 16.0f;
   io.Fonts->AddFontFromFileTTF(fontFilename.c_str(), baseFontSize);

   // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly
   constexpr auto iconFontSize = baseFontSize * 2.0f / 3.0f;

   // NOLINTNEXTLINE
   static constexpr std::array< ImWchar, 3 > icons_ranges = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
   ImFontConfig icons_config;
   icons_config.MergeMode = true;
   icons_config.PixelSnapH = true;
   icons_config.GlyphMinAdvanceX = iconFontSize;

   io.Fonts->AddFontFromFileTTF((FONTS_DIR / FONT_ICON_FILE_NAME_FAS).string().c_str(),
                                iconFontSize, &icons_config, icons_ranges.data());
   // Create font texture
   unsigned char* fontData = nullptr;
   int32_t texWidth = 0;
   int32_t texHeight = 0;
   io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);

   const auto image = renderer::Texture::CreateImage(
      static_cast< uint32_t >(texWidth), static_cast< uint32_t >(texHeight), 1,
      VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
   renderer::EditorData::fontImage_ = image.textureImage_;
   renderer::EditorData::fontMemory_ = image.textureImageMemory_;

   renderer::EditorData::m_fontView = renderer::Texture::CreateImageView(
      renderer::EditorData::fontImage_, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1);

   renderer::Texture::TransitionImageLayout(renderer::EditorData::fontImage_,
                                            VK_IMAGE_LAYOUT_UNDEFINED,
                                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);

   renderer::Texture::CopyBufferToImage(renderer::EditorData::fontImage_,
                                        static_cast< uint32_t >(texWidth),
                                        static_cast< uint32_t >(texHeight), fontData);

   renderer::Texture::TransitionImageLayout(renderer::EditorData::fontImage_,
                                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

   // Font texture Sampler
   renderer::EditorData::sampler_ = renderer::Texture::CreateSampler();

   // Descriptor pool
   VkDescriptorPoolSize descriptorPoolSize{};
   descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   descriptorPoolSize.descriptorCount = 1;
   std::vector< VkDescriptorPoolSize > poolSizes = {descriptorPoolSize};

   VkDescriptorPoolCreateInfo descriptorPoolInfo{};
   descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   descriptorPoolInfo.poolSizeCount = static_cast< uint32_t >(poolSizes.size());
   descriptorPoolInfo.pPoolSizes = poolSizes.data();
   descriptorPoolInfo.maxSets = renderer::MAX_NUM_TEXTURES;

   renderer::vk_check_error(vkCreateDescriptorPool(renderer::Data::vk_device, &descriptorPoolInfo,
                                                   nullptr, &renderer::EditorData::descriptorPool_),
                            "vkCreateDescriptorPool failed for UI setup!");

   // Descriptor set layout
   VkDescriptorSetLayoutBinding setLayoutBinding{};
   setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   setLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
   setLayoutBinding.binding = 0;
   setLayoutBinding.descriptorCount = 1;

   std::vector< VkDescriptorSetLayoutBinding > setLayoutBindings = {setLayoutBinding};

   VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
   descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   descriptorSetLayoutCreateInfo.pBindings = setLayoutBindings.data();
   descriptorSetLayoutCreateInfo.bindingCount = static_cast< uint32_t >(setLayoutBindings.size());


   renderer::vk_check_error(
      vkCreateDescriptorSetLayout(renderer::Data::vk_device, &descriptorSetLayoutCreateInfo,
                                  nullptr, &renderer::EditorData::descriptorSetLayout_),
      "vkCreateDescriptorSetLayout failed for UI setup!");

   // Descriptor set
   VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
   descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   descriptorSetAllocateInfo.descriptorPool = renderer::EditorData::descriptorPool_;
   descriptorSetAllocateInfo.pSetLayouts = &renderer::EditorData::descriptorSetLayout_;
   descriptorSetAllocateInfo.descriptorSetCount = 1;


   renderer::vk_check_error(vkAllocateDescriptorSets(renderer::Data::vk_device,
                                                     &descriptorSetAllocateInfo,
                                                     &renderer::EditorData::descriptorSet_),
                            "vkAllocateDescriptorSets failed for UI setup!");

   VkDescriptorImageInfo descriptorImageInfo{};
   descriptorImageInfo.sampler = renderer::EditorData::sampler_;
   descriptorImageInfo.imageView = renderer::EditorData::m_fontView;
   descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

   VkWriteDescriptorSet writeDescriptorSet{};
   writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   writeDescriptorSet.dstSet = renderer::EditorData::descriptorSet_;
   writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   writeDescriptorSet.dstBinding = 0;
   writeDescriptorSet.pImageInfo = &descriptorImageInfo;
   writeDescriptorSet.descriptorCount = 1;

   std::vector< VkWriteDescriptorSet > writeDescriptorSets = {writeDescriptorSet};
   vkUpdateDescriptorSets(renderer::Data::vk_device,
                          static_cast< uint32_t >(writeDescriptorSets.size()),
                          writeDescriptorSets.data(), 0, nullptr);


   renderer::EditorData::vertexBuffer_.resize(renderer::MAX_FRAMES_IN_FLIGHT);
   renderer::EditorData::indexBuffer_.resize(renderer::MAX_FRAMES_IN_FLIGHT);
   renderer::EditorData::vertexCount_.resize(renderer::MAX_FRAMES_IN_FLIGHT);
   renderer::EditorData::indexCount_.resize(renderer::MAX_FRAMES_IN_FLIGHT);
}

void
EditorGUI::PreparePipeline()
{
   const auto& renderData = renderer::Data::renderData_.at(renderer::GetCurrentlyBoundType());
   // Pipeline layout
   // Push constants for UI rendering parameters
   VkPushConstantRange pushConstantRange{};
   pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
   pushConstantRange.offset = 0;
   pushConstantRange.size = sizeof(renderer::PushConstBlock);

   VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
   pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   pipelineLayoutCreateInfo.setLayoutCount = 1;
   pipelineLayoutCreateInfo.pSetLayouts = &renderer::EditorData::descriptorSetLayout_;

   pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
   pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
   renderer::vk_check_error(vkCreatePipelineLayout(renderer::Data::vk_device,
                                                   &pipelineLayoutCreateInfo, nullptr,
                                                   &renderer::EditorData::pipelineLayout_),
                            "");

   // Setup graphics pipeline for UI rendering
   VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
   pipelineInputAssemblyStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
   pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
   pipelineInputAssemblyStateCreateInfo.flags = 0;
   pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

   VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
   pipelineRasterizationStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
   pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
   pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
   pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
   pipelineRasterizationStateCreateInfo.flags = 0;
   pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
   pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;


   // Enable blending
   VkPipelineColorBlendAttachmentState blendAttachmentState{};
   blendAttachmentState.blendEnable = VK_TRUE;
   blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                         | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
   blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
   blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
   blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
   blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
   blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
   blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

   VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
   pipelineColorBlendStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
   pipelineColorBlendStateCreateInfo.attachmentCount = 1;
   pipelineColorBlendStateCreateInfo.pAttachments = &blendAttachmentState;

   VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
   pipelineDepthStencilStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
   pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
   pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
   pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_ALWAYS;
   pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;

   VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
   pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
   pipelineViewportStateCreateInfo.viewportCount = 1;
   pipelineViewportStateCreateInfo.scissorCount = 1;
   pipelineViewportStateCreateInfo.flags = 0;

   VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
   pipelineMultisampleStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
   pipelineMultisampleStateCreateInfo.rasterizationSamples = renderer::Data::msaaSamples;
   pipelineMultisampleStateCreateInfo.flags = 0;


   std::vector< VkDynamicState > dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT,
                                                        VK_DYNAMIC_STATE_SCISSOR};

   VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
   pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
   pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();
   pipelineDynamicStateCreateInfo.dynamicStateCount =
      static_cast< uint32_t >(dynamicStateEnables.size());
   pipelineDynamicStateCreateInfo.flags = 0;

   auto [vertexInfo, fragmentInfo] =
      renderer::VulkanShader::CreateShader(renderer::Data::vk_device, "ui.vert.spv", "ui.frag.spv");
   std::array< VkPipelineShaderStageCreateInfo, 2 > shaderStages = {vertexInfo.shaderInfo,
                                                                    fragmentInfo.shaderInfo};

   VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
   pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
   pipelineCreateInfo.layout = renderer::EditorData::pipelineLayout_;
   pipelineCreateInfo.renderPass = renderData.renderPass;
   pipelineCreateInfo.flags = 0;
   pipelineCreateInfo.basePipelineIndex = -1;
   pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
   pipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
   pipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
   pipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
   pipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
   pipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
   pipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
   pipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
   pipelineCreateInfo.stageCount = 2;
   pipelineCreateInfo.pStages = shaderStages.data();
   pipelineCreateInfo.subpass = renderer::EditorData::subpass_;

   // Vertex bindings an attributes based on ImGui vertex definition
   VkVertexInputBindingDescription vInputBindDescription{};
   vInputBindDescription.binding = 0;
   vInputBindDescription.stride = sizeof(ImDrawVert);
   vInputBindDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

   std::vector< VkVertexInputBindingDescription > vertexInputBindings = {vInputBindDescription};

   VkVertexInputAttributeDescription pos{};
   pos.location = 0;
   pos.binding = 0;
   pos.format = VK_FORMAT_R32G32_SFLOAT;
   pos.offset = offsetof(ImDrawVert, pos);

   VkVertexInputAttributeDescription uv{};
   uv.location = 1;
   uv.binding = 0;
   uv.format = VK_FORMAT_R32G32_SFLOAT;
   uv.offset = offsetof(ImDrawVert, uv);

   VkVertexInputAttributeDescription color{};
   color.location = 2;
   color.binding = 0;
   color.format = VK_FORMAT_R8G8B8A8_UNORM;
   color.offset = offsetof(ImDrawVert, col);


   std::vector< VkVertexInputAttributeDescription > vertexInputAttributes = {
      pos,   // Location 0: Position
      uv,    // Location 1: UV
      color, // Location 2: Color
   };

   VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
   pipelineVertexInputStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

   pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount =
      static_cast< uint32_t >(vertexInputBindings.size());
   pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vertexInputBindings.data();
   pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount =
      static_cast< uint32_t >(vertexInputAttributes.size());
   pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributes.data();

   pipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;

   renderer::vk_check_error(
      vkCreateGraphicsPipelines(renderer::Data::vk_device, renderData.pipelineCache, 1,
                                &pipelineCreateInfo, nullptr, &renderer::EditorData::pipeline_),
      "");
}

} // namespace looper
