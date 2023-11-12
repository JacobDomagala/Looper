#include "editor_gui.hpp"
#include "animatable.hpp"
#include "editor.hpp"
#include "game_object.hpp"
#include "helpers.hpp"
#include "icons.hpp"
#include "renderer/renderer.hpp"
#include "renderer/shader.hpp"
#include "renderer/texture.hpp"
#include "renderer/types.hpp"
#include "renderer/vulkan_common.hpp"
#include "types.hpp"
#include "utils/file_manager.hpp"

#include <GLFW/glfw3.h>
#include <fmt/format.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <cstdint>


namespace looper {

std::unordered_map< renderer::TextureID, VkDescriptorSet > textureDescriptors = {};

VkDescriptorSet
GetDescriptor(renderer::TextureID id, VkDescriptorPool descriptorPool,
              VkDescriptorSetLayout descriptorSetLayout)
{
   const auto desc = textureDescriptors.find(id);

   VkDescriptorSet descriptor{};
   if (desc != textureDescriptors.end())
   {
      descriptor = desc->second;
   }
   else
   {
      auto [view, sampler] = renderer::TextureLibrary::GetTexture(id)->GetImageViewAndSampler();
      descriptor = renderer::Texture::CreateDescriptorSet(sampler, view,
                                                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                          descriptorPool, descriptorSetLayout);

      textureDescriptors[id] = descriptor;
   }

   return descriptor;
}

EditorGUI::EditorGUI(Editor& parent) : parent_(parent)
{
}

void
EditorGUI::KeyCallback(KeyEvent& event)
{
   // Editor shoould have priority with KeyCallback
   parent_.KeyCallback(event);
   if (!event.handled_)
   {
      auto* window = parent_.GetWindow().GetWindowHandle();
      ImGuiIO& io = ImGui::GetIO();
      io.AddKeyEvent(ImGuiMod_Ctrl,
                     (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
                        || (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS));
      io.AddKeyEvent(ImGuiMod_Shift,
                     (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                        || (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS));
      io.AddKeyEvent(ImGuiMod_Alt, (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
                                      || (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS));
      io.AddKeyEvent(ImGuiMod_Super,
                     (glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS)
                        || (glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS));

      const auto imguiKey = KeyToImGuiKey(event.key_);
      io.AddKeyEvent(imguiKey, (event.action_ == GLFW_PRESS));

      if ((event.key_ == GLFW_KEY_ESCAPE) and (event.action_ == GLFW_PRESS))
      {
         exitPushed_ = not exitPushed_;
         event.handled_ = true;
      }
   }
}

void
EditorGUI::CharCallback(CharEvent& event)
{
   ImGuiIO& io = ImGui::GetIO();
   io.AddInputCharacter(event.key_);
}

void
EditorGUI::MouseButtonCallback(MouseButtonEvent& event)
{
   ImGuiIO& io = ImGui::GetIO();

   io.MouseDown[0] = (event.button_ == GLFW_MOUSE_BUTTON_1) && event.action_;
   io.MouseDown[1] = (event.button_ == GLFW_MOUSE_BUTTON_2) && event.action_;
}

void
EditorGUI::CursorPositionCallback(CursorPositionEvent& event)
{
   ImGuiIO& io = ImGui::GetIO();
   io.MousePos = ImVec2(static_cast< float >(event.xPos_), static_cast< float >(event.yPos_));
}

void
EditorGUI::MouseScrollCallback(MouseScrollEvent& /*event*/)
{
}

void
EditorGUI::Init()
{
   InputManager::RegisterForInput(parent_.GetWindow().GetWindowHandle(), this);

   // Setup Dear ImGui context
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO& io = ImGui::GetIO();
   io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls NOLINT

   SetStyle();

   PrepareResources();
   PreparePipeline();
}

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
   if ((vertexBuffer_[currentFrame].m_buffer == VK_NULL_HANDLE)
       || (vertexCount_[currentFrame] != imDrawData->TotalVtxCount))
   {
      vertexBuffer_[currentFrame].Unmap();
      vertexBuffer_[currentFrame].Destroy();

      vertexBuffer_[currentFrame] = renderer::Buffer::CreateBuffer(
         vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      vertexCount_[currentFrame] = imDrawData->TotalVtxCount;
      vertexBuffer_[currentFrame].Map();
   }

   // Index buffer
   if ((indexBuffer_[currentFrame].m_buffer == VK_NULL_HANDLE)
       || (indexCount_[currentFrame] < imDrawData->TotalIdxCount))
   {
      indexBuffer_[currentFrame].Unmap();
      indexBuffer_[currentFrame].Destroy();

      indexBuffer_[currentFrame] = renderer::Buffer::CreateBuffer(
         indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
      indexCount_[currentFrame] = imDrawData->TotalIdxCount;
      indexBuffer_[currentFrame].Map();
   }

   // Upload data
   auto* vtxDst = static_cast< ImDrawVert* >(vertexBuffer_[currentFrame].m_mappedMemory);
   auto* idxDst = static_cast< ImDrawIdx* >(indexBuffer_[currentFrame].m_mappedMemory);

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
   vertexBuffer_[currentFrame].Flush();
   indexBuffer_[currentFrame].Flush();
}

void
EditorGUI::Render(VkCommandBuffer commandBuffer)
{
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

   vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);

   pushConstant_.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
   pushConstant_.translate = glm::vec2(-1.0f);
   vkCmdPushConstants(commandBuffer, pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0,
                      sizeof(PushConstBlock), &pushConstant_);

   std::array< VkDeviceSize, 1 > offsets = {0};
   vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer_[currentFrame].m_buffer,
                          offsets.data());
   vkCmdBindIndexBuffer(commandBuffer, indexBuffer_[currentFrame].m_buffer, 0,
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
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_,
                                    0, 1, desc_set.data(), 0, nullptr);
         }
         else
         {
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_,
                                    0, 1, &descriptorSet_, 0, nullptr);
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

   std::tie(fontImage_, fontMemory_) = renderer::Texture::CreateImage(
      static_cast< uint32_t >(texWidth), static_cast< uint32_t >(texHeight), 1,
      VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

   m_fontView = renderer::Texture::CreateImageView(fontImage_, VK_FORMAT_R8G8B8A8_UNORM,
                                                   VK_IMAGE_ASPECT_COLOR_BIT, 1);

   renderer::Texture::TransitionImageLayout(fontImage_, VK_IMAGE_LAYOUT_UNDEFINED,
                                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);

   renderer::Texture::CopyBufferToImage(fontImage_, static_cast< uint32_t >(texWidth),
                                        static_cast< uint32_t >(texHeight), fontData);

   renderer::Texture::TransitionImageLayout(fontImage_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

   // Font texture Sampler
   sampler_ = renderer::Texture::CreateSampler();

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
                                                   nullptr, &descriptorPool_),
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


   renderer::vk_check_error(vkCreateDescriptorSetLayout(renderer::Data::vk_device,
                                                        &descriptorSetLayoutCreateInfo, nullptr,
                                                        &descriptorSetLayout_),
                            "vkCreateDescriptorSetLayout failed for UI setup!");

   // Descriptor set
   VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
   descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   descriptorSetAllocateInfo.descriptorPool = descriptorPool_;
   descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout_;
   descriptorSetAllocateInfo.descriptorSetCount = 1;


   renderer::vk_check_error(vkAllocateDescriptorSets(renderer::Data::vk_device,
                                                     &descriptorSetAllocateInfo, &descriptorSet_),
                            "vkAllocateDescriptorSets failed for UI setup!");

   VkDescriptorImageInfo descriptorImageInfo{};
   descriptorImageInfo.sampler = sampler_;
   descriptorImageInfo.imageView = m_fontView;
   descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

   VkWriteDescriptorSet writeDescriptorSet{};
   writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   writeDescriptorSet.dstSet = descriptorSet_;
   writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   writeDescriptorSet.dstBinding = 0;
   writeDescriptorSet.pImageInfo = &descriptorImageInfo;
   writeDescriptorSet.descriptorCount = 1;

   std::vector< VkWriteDescriptorSet > writeDescriptorSets = {writeDescriptorSet};
   vkUpdateDescriptorSets(renderer::Data::vk_device,
                          static_cast< uint32_t >(writeDescriptorSets.size()),
                          writeDescriptorSets.data(), 0, nullptr);


   vertexBuffer_.resize(renderer::MAX_FRAMES_IN_FLIGHT);
   indexBuffer_.resize(renderer::MAX_FRAMES_IN_FLIGHT);
   vertexCount_.resize(renderer::MAX_FRAMES_IN_FLIGHT);
   indexCount_.resize(renderer::MAX_FRAMES_IN_FLIGHT);
}

void
EditorGUI::PreparePipeline()
{
   const auto& renderData =
      renderer::Data::renderData_.at(renderer::VulkanRenderer::GetCurrentlyBoundType());
   // Pipeline layout
   // Push constants for UI rendering parameters
   VkPushConstantRange pushConstantRange{};
   pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
   pushConstantRange.offset = 0;
   pushConstantRange.size = sizeof(PushConstBlock);

   VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
   pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   pipelineLayoutCreateInfo.setLayoutCount = 1;
   pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout_;

   pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
   pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
   renderer::vk_check_error(vkCreatePipelineLayout(renderer::Data::vk_device,
                                                   &pipelineLayoutCreateInfo, nullptr,
                                                   &pipelineLayout_),
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
   pipelineCreateInfo.layout = pipelineLayout_;
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
   pipelineCreateInfo.subpass = subpass_;

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

   renderer::vk_check_error(vkCreateGraphicsPipelines(renderer::Data::vk_device,
                                                      renderData.pipelineCache, 1,
                                                      &pipelineCreateInfo, nullptr, &pipeline_),
                            "");
}

void
EditorGUI::Shutdown()
{
   ImGui::DestroyContext();
}

bool
EditorGUI::IsBlockingEvents()
{
   const ImGuiIO& io = ImGui::GetIO();
   return io.WantCaptureMouse || io.WantTextInput;
}

void
EditorGUI::RenderCreateNewLevelWindow()
{
   const auto halfSize = windowSize_ / 2.0f;
   std::unordered_map< std::string, glm::ivec2 > sizes = {{"Small", glm::ivec2{4096, 4096}},
                                                          {"Medium", glm::ivec2{16384, 16384}},
                                                          {"Large", glm::ivec2{65536, 65536}}};
   static glm::ivec2 size = {1024, 1024};
   static std::string name = "DummyLevelName";
   static std::string currentSize = "Small";

   ImGui::SetNextWindowPos({halfSize.x - 160, halfSize.y - 60});
   ImGui::SetNextWindowSize({300, 180});
   ImGui::Begin("Create New", nullptr, ImGuiWindowFlags_NoResize);

   ImGui::Text("Size:");
   BlankLine(ImVec2(2.0f, 0.0f));
   ImGui::SameLine();
   ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.6f);

   const auto items = std::to_array< std::string >({"Small", "Medium", "Large"});
   // The second parameter is the label previewed before opening the combo.
   if (ImGui::BeginCombo("##combo", currentSize.c_str()))
   {
      for (const auto& item : items)
      {
         if (ImGui::Selectable(item.c_str()))
         {
            size = sizes.at(item);
            currentSize = item;
         }
      }
      ImGui::EndCombo();
   }

   ImGui::Dummy(ImVec2(0.0f, 5.0f));

   ImGui::Text("Name:");
   ImGui::Dummy(ImVec2(2.0f, 0.0f));
   ImGui::SameLine();
   ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.6f);
   ImGui::InputText("##Name", name.data(), name.capacity() + 1);

   ImGui::Dummy(ImVec2(0.0f, 5.0f));
   ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 10.0f, 0.0f));
   ImGui::SameLine();
   // ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 300) / 2);
   if (ImGui::Button("Create", {ImGui::GetWindowWidth() / 3.0f, 35}))
   {
      parent_.CreateLevel(name, size);
      createPushed_ = false;
   }
   ImGui::SameLine();
   ImGui::Dummy(ImVec2(2.0f, 0.0f));
   ImGui::SameLine();
   if (ImGui::Button("Cancel", {ImGui::GetWindowWidth() / 3.0f, 35}))
   {
      createPushed_ = false;
   }

   ImGui::End();
}

void
EditorGUI::RenderExitWindow()
{
   const auto halfSize = windowSize_ / 2.0f;

   ImGui::SetNextWindowPos({halfSize.x - 160, halfSize.y - 60});
   ImGui::SetNextWindowSize({240, 120});
   ImGui::Begin("Exit", nullptr, ImGuiWindowFlags_NoResize);

   ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.6f);

   ImGui::Text("Do you want to exit?");
   ImGui::Dummy(ImVec2(2.0f, 0.0f));

   ImGui::Dummy(ImVec2(0.0f, 5.0f));
   ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 10.0f, 0.0f));
   ImGui::SameLine();
   // ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 300) / 2);
   if (ImGui::Button("Exit", {ImGui::GetWindowWidth() / 3.0f, 35}))
   {
      parent_.Shutdown();
   }
   ImGui::SameLine();
   ImGui::Dummy(ImVec2(2.0f, 0.0f));
   ImGui::SameLine();
   if (ImGui::Button("Cancel", {ImGui::GetWindowWidth() / 3.0f, 35}))
   {
      exitPushed_ = false;
   }

   ImGui::End();
}

void
EditorGUI::RenderMainPanel()
{
   ImGui::SetNextWindowPos({0, 0});
   ImGui::SetNextWindowSize(ImVec2(windowWidth_, toolsWindowHeight_));
   ImGui::Begin("Tools");

   if (ImGui::BeginTable("MainTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
   {
      CreateActionRow(
         [this] {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.0f, 0.5f, 0.0f, 0.8f});
            ImGui::BeginDisabled(currentLevel_ == nullptr);
            if (ImGui::Button(ICON_FA_PLAY "Play", ImVec2(-FLT_MIN, -FLT_MIN)))
            {
               parent_.AddToWorkQueue([this] { parent_.PlayLevel(); });
            }

            ImGui::PopStyleColor(1);
         },

         [this] {
            if (ImGui::Button("Save", ImVec2(-FLT_MIN, -FLT_MIN)))
            {
               auto levelName =
                  FileManager::FileDialog(LEVELS_DIR, {{"DGame Level file", "dgl"}}, true);
               if (!levelName.empty())
               {
                  parent_.AddToWorkQueue([this, levelName] { parent_.SaveLevel(levelName); });
               }
            }
            ImGui::EndDisabled();
         },

         [this] {
            if (ImGui::Button("Load", ImVec2(-FLT_MIN, -FLT_MIN)))
            {
               auto levelName =
                  FileManager::FileDialog(LEVELS_DIR, {{"DGame Level file", "dgl"}}, false);
               if (!levelName.empty())
               {
                  parent_.AddToWorkQueue([this, levelName] { parent_.LoadLevel(levelName); });
               }
            }
         },

         [this] {
            if (ImGui::Button("Create", ImVec2(-FLT_MIN, -FLT_MIN)) or createPushed_)
            {
               createPushed_ = true;
               RenderCreateNewLevelWindow();
            }
         });

      ImGui::EndTable();
   }


   ImGui::End();
}

void
EditorGUI::RenderLevelMenu() // NOLINT
{
   ImGui::SetNextWindowPos({0, toolsWindowHeight_});
   ImGui::SetNextWindowSize(ImVec2(windowWidth_, levelWindowHeight_));
   ImGui::Begin("Level");
   ImGui::SetNextItemOpen(true);
   if (ImGui::CollapsingHeader("General"))
   {
      if (ImGui::BeginTable("LevelTable", 2))
      {
         CreateRow("Size", fmt::format("{:.0f}, {:.0f}", currentLevel_->GetSprite().GetSize().x,
                                       currentLevel_->GetSprite().GetSize().y));

         CreateActionRowLabel("Render grid", [this] {
            auto [drawGrid, gridSize] = parent_.GetGridData();
            if (ImGui::Checkbox("##Render grid", &drawGrid))
            {
               parent_.SetGridData(drawGrid, gridSize);
            }
         });

         CreateActionRowLabel("Render collision", [this] {
            auto renderPathfinderNodes = parent_.GetRenderNodes();
            if (ImGui::Checkbox("##Render collision", &renderPathfinderNodes))
            {
               parent_.RenderNodes(renderPathfinderNodes);
            }
         });

         CreateActionRowLabel("RenderLayer", [this] {
            const auto items = std::to_array< std::string >(
               {"All", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"});
            const auto layer = parent_.GetRenderLayerToDraw();
            if (ImGui::BeginCombo(
                   "##combo", fmt::format("{}", layer == -1 ? "All" : std::to_string(layer)).c_str()))
            {
               for (const auto& item : items)
               {
                  if (ImGui::Selectable(item.c_str()))
                  {
                     parent_.SetRenderLayerToDraw(item == "All" ? -1 : std::stoi(item));
                  }
               }
               ImGui::EndCombo();
            }
         });

         ImGui::EndTable();
      }
   }

   BlankLine();

   ImGui::SetNextItemOpen(true);
   if (ImGui::CollapsingHeader("Objects"))
   {
      const auto& gameObjects = currentLevel_->GetObjects();

      const auto items = std::to_array< std::string >({"Enemy", "Player", "Object"});
      ImGui::SetNextItemWidth(windowWidth_ * 0.95f);

      // The second parameter is the label previewed before opening the combo.
      if (ImGui::BeginCombo("##combo", "Add"))
      {
         for (const auto& item : items)
         {
            if (ImGui::Selectable(item.c_str()))
            {
               parent_.AddToWorkQueue(
                  [this, item] { parent_.AddGameObject(Object::GetTypeFromString(item)); });
            }
         }
         ImGui::EndCombo();
      }

      ImGui::BeginChild("Loaded Objects", {0, 200}, true);

      for (const auto& object : gameObjects)
      {
         auto label = fmt::format("[{}] {} ({:.2f}, {:.2f})", object->GetTypeString().c_str(),
                                  object->GetName().c_str(), object->GetPosition().x,
                                  object->GetPosition().y);

         if (ImGui::Selectable(label.c_str()))
         {
            parent_.GetCamera().SetCameraAtPosition(object->GetPosition());
            parent_.HandleGameObjectSelected(object, true);
         }
      }

      ImGui::EndChild();
   }

   BlankLine();

   ImGui::SetNextItemOpen(true);
   if (ImGui::CollapsingHeader("Debug"))
   {
      if (ImGui::BeginTable("DebugTable", 2))
      {
         CreateRow("FPS", fmt::format("{}", parent_.GetFramesLastSecond()));
         CreateRow("Frame time",
                   fmt::format("{:.2f}ms", parent_.GetFrameTime().GetMilliseconds().count()));
         CreateRow("UI time",
                   fmt::format("{:.2f}ms", parent_.GetUpdateUITime().GetMilliseconds().count()));
         CreateRow("Render time",
                   fmt::format("{:.2f}ms", parent_.GetRenderTime().GetMilliseconds().count()));
         const auto cameraPos = parent_.GetCamera().GetPosition();
         CreateRow("Camera Position", fmt::format("{}", static_cast< glm::vec2 >(cameraPos)));
         CreateRow("Camera Zoom", fmt::format("{:.1f}", parent_.GetCamera().GetZoomLevel()));

         CreateRow("Camera Rotation", fmt::format("{:.1f}", parent_.GetCamera().GetRotation()));

         const auto cursorPos = parent_.ScreenToGlobal(InputManager::GetMousePos());
         CreateRow("Cursor Position", fmt::format("{}", cursorPos));

         auto& pathfinder = parent_.GetLevel().GetPathfinder();

         const auto nodeID = pathfinder.GetNodeIDFromPosition(cursorPos);
         const auto curNode = nodeID != -1 ? pathfinder.GetNodeFromID(nodeID) : Node{};

         CreateRow("Cursor on TileID", fmt::format("{}", curNode.id_));
         CreateRow("Cursor on Coords", fmt::format("({}, {})", curNode.xPos_, curNode.yPos_));

         ImGui::EndTable();
      }
   }


   ImGui::End();
}

void
EditorGUI::RenderGameObjectMenu() // NOLINT
{
   ImGui::SetNextWindowPos({windowSize_.x - windowWidth_, 0});
   ImGui::SetNextWindowSize(ImVec2(windowWidth_, gameObjectWindowHeight_));
   ImGui::Begin("Game Object");
   ImGui::SetNextItemOpen(true);

   if (ImGui::CollapsingHeader("General"))
   {
      DrawWidget("Name", [this]() {
         auto name = currentlySelectedGameObject_->GetName();
         const auto nameLength = 20;
         name.resize(nameLength);
         if (ImGui::InputText("##Name", name.data(), nameLength))
         {
            currentlySelectedGameObject_->SetName(name);
         }
      });

      if (ImGui::BeginTable("ObjectTable", 2))
      {
         CreateActionRowLabel("RenderLayer", [this] {
            const auto items = std::to_array< std::string >(
               {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"});
            if (ImGui::BeginCombo(
                   "##combo",
                   fmt::format("{}",
                               currentlySelectedGameObject_->GetSprite().GetRenderInfo().layer)
                      .c_str()))
            {
               for (const auto& item : items)
               {
                  if (ImGui::Selectable(item.c_str()))
                  {
                     parent_.AddToWorkQueue([this, item] {
                        const auto layer = static_cast< uint32_t >(std::stoi(item));
                        const auto oldLayer =
                           currentlySelectedGameObject_->GetSprite().GetRenderInfo().layer;
                        currentlySelectedGameObject_->GetSprite().ChangeRenderLayer(layer);

                        renderer::VulkanRenderer::SetupVertexBuffer(oldLayer);
                        renderer::VulkanRenderer::SetupVertexBuffer(layer);
                     });
                  }
               }
               ImGui::EndCombo();
            }
         });
         CreateRow("Type", fmt::format("{}", currentlySelectedGameObject_->GetTypeString()));
         CreateRow("ID", fmt::format("{}", currentlySelectedGameObject_->GetID()));
         CreateActionRowLabel("Has Collision", [this] {
            auto collision = currentlySelectedGameObject_->GetHasCollision();
            if (ImGui::Checkbox("##Has Collision", &collision))
            {
               currentlySelectedGameObject_->SetHasCollision(collision);
            }
         });

         ImGui::EndTable();
      }
   }

   BlankLine();

   ImGui::SetNextItemOpen(true);
   if (ImGui::CollapsingHeader("Transform"))
   {
      DrawWidget("Position", [this]() {
         auto objectPosition = currentlySelectedGameObject_->GetSprite().GetPosition();
         ImGui::InputFloat2("##Position", &objectPosition.x);
      });

      DrawWidget("Size", [this]() {
         auto sprite_size = currentlySelectedGameObject_->GetSprite().GetSize();
         if (ImGui::SliderFloat2("##Size", &sprite_size.x, 10, 1000))
         {
            currentlySelectedGameObject_->SetSize(sprite_size);
         }
      });

      DrawWidget("Rotate", [this]() {
         auto rotation = currentlySelectedGameObject_->GetSprite().GetRotation(
            renderer::Sprite::RotationType::DEGREES);
         if (ImGui::SliderFloat("##Rotate", &rotation,
                                glm::degrees(renderer::Sprite::ROTATION_RANGE.first),
                                glm::degrees(renderer::Sprite::ROTATION_RANGE.second)))
         {
            currentlySelectedGameObject_->Rotate(glm::radians(rotation));
         }
      });
   }

   BlankLine();

   ImGui::SetNextItemOpen(true);
   if (ImGui::CollapsingHeader("Shader"))
   {
      const auto sectionSize = ImGui::GetContentRegionAvail();
      ImGui::Image(static_cast< ImTextureID >(
                      GetDescriptor(currentlySelectedGameObject_->GetSprite().GetTexture()->GetID(),
                                    descriptorPool_, descriptorSetLayout_)),
                   {sectionSize.x, sectionSize.x});

      DrawWidget("Texture", [this, sectionSize]() {
         auto& sprite = currentlySelectedGameObject_->GetSprite();
         const float fullWidth = sectionSize.x;
         const float inputTextWidth = fullWidth * 0.90f;
         const float buttonWidth = fullWidth * 0.10f;

         ImGui::PushItemWidth(inputTextWidth);
         ImGui::InputText("##Texture", sprite.GetTextureName().data(),
                          sprite.GetTextureName().size(), ImGuiInputTextFlags_ReadOnly);
         ImGui::PopItemWidth(); // Always pair a Push call with a Pop

         ImGui::SameLine();

         ImGui::PushItemWidth(buttonWidth);
         if (ImGui::Button(ICON_FA_PENCIL ""))
         {
            auto textureName = FileManager::FileDialog(
               IMAGES_DIR, {{"PNG texture", "png"}, {"JPEG texture", "jpg"}}, false);
            if (!textureName.empty())
            {
               sprite.SetTextureFromFile(textureName);
            }
         }
         ImGui::PopItemWidth(); // Always pair a Push call with a Pop
      });
   }

   if (currentlySelectedGameObject_->GetType() == ObjectType::ENEMY)
   {
      const auto animatablePtr =
         std::dynamic_pointer_cast< Animatable >(currentlySelectedGameObject_);

      BlankLine();

      ImGui::SetNextItemOpen(true);
      if (ImGui::CollapsingHeader("Animation"))
      {
         DrawWidget("Type", [animatablePtr]() {
            if (ImGui::RadioButton("Loop", animatablePtr->GetAnimationType()
                                              == Animatable::ANIMATION_TYPE::LOOP))
            {
               animatablePtr->SetAnimationType(Animatable::ANIMATION_TYPE::LOOP);
            }

            ImGui::SameLine();
            if (ImGui::RadioButton("Reversal", animatablePtr->GetAnimationType()
                                                  == Animatable::ANIMATION_TYPE::REVERSABLE))
            {
               animatablePtr->SetAnimationType(Animatable::ANIMATION_TYPE::REVERSABLE);
            }
         });

         bool animationVisible = animatablePtr->GetRenderAnimationSteps(); // NOLINT
         if (ImGui::Checkbox("Animation points visible", &animationVisible))
         {
            parent_.SetRenderAnimationPoints(animationVisible);
         }

         if (ImGui::Button("Animate"))
         {
            parent_.ToggleAnimateObject();
         }

         static float timer = 0.0f; // NOLINT
         const auto animationDuration =
            time::Timer::ConvertToMs(animatablePtr->GetAnimationDuration()).count();
         if (parent_.IsObjectAnimated())
         {
            timer += static_cast< float >(parent_.GetDeltaTime().count());
            timer = glm::min(animationDuration, timer);
         }

         ImGui::SameLine();
         if (ImGui::SliderFloat("##", &timer, 0.0f, animationDuration, "%.3f ms"))
         {
            currentlySelectedGameObject_->GetSprite().SetTranslateValue(glm::vec3(
               animatablePtr->SetAnimation(time::milliseconds(static_cast< uint64_t >(timer))),
               0.0f));
         }

         auto& animationPoints = animatablePtr->GetAnimationKeypoints();
         auto newNodePosition = currentlySelectedGameObject_->GetPosition();
         ImGui::BeginChild("Animation Points", {0, 100}, true);
         if (ImGui::BeginTable("AnimationPointTable", 2))
         {
            auto contentWidth = ImGui::GetContentRegionAvail().x;
            ImGui::TableSetupColumn("Column 1", ImGuiTableColumnFlags_WidthStretch,
                                    contentWidth * 0.95f);
            ImGui::TableSetupColumn("Column 2", ImGuiTableColumnFlags_WidthStretch,
                                    contentWidth * 0.05f);

            for (uint32_t i = 0; i < animationPoints.size(); ++i) // NOLINT
            {
               const auto& node = animationPoints[i];
               const auto label = fmt::format("[{}] Time={}s", i, node.m_timeDuration.count());

               ImGui::TableNextRow();
               ImGui::TableNextColumn();
               if (ImGui::Selectable(label.c_str()))
               {
                  parent_.SelectAnimationPoint(node);
               }
               ImGui::TableNextColumn();

               ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0f, 0.0f, 0.0f, 1.0f});
               if (ImGui::Selectable(fmt::format("{}##{}", ICON_FA_XMARK, i).c_str()))
               {
                  parent_.AddToWorkQueue([this, nodeID = node.GetID()] {
                     parent_.ActionOnObject(Editor::ACTION::REMOVE, nodeID);
                  });
               }
               ImGui::PopStyleColor(1);

               newNodePosition = node.m_end;
            }
         }
         ImGui::EndTable();

         if (ImGui::Button("New"))
         {
            parent_.AddToWorkQueue(
               [this, newNodePosition] { parent_.AddAnimationPoint(newNodePosition); });
         }
         ImGui::EndChild();

         const auto selectedID = parent_.GetSelectedEditorObject();
         if (Object::GetTypeFromID(selectedID) == ObjectType::ANIMATION_POINT)
         {
            BlankLine();
            ImGui::SetNextItemOpen(true);
            if (ImGui::CollapsingHeader("Selected point"))
            {
               auto& node =
                  dynamic_cast< AnimationPoint& >(parent_.GetLevel().GetObjectRef(selectedID));

               if (ImGui::BeginTable("AnimationPointTable", 2))
               {
                  const auto it =
                     stl::find_if(animationPoints, [selectedID](const auto& animationPoint) {
                        return animationPoint.GetID() == selectedID;
                     });
                  const auto idx = std::distance(animationPoints.begin(), it);

                  CreateRow("Idx", fmt::format("{}", idx));
                  CreateRow("Position", fmt::format("{:.2f},{:.2f}", node.m_end.x, node.m_end.y));

                  ImGui::EndTable();
               }

               DrawWidget(fmt::format("Duration (sec)", node.m_end.x, node.m_end.y), [&node]() {
                  auto seconds = static_cast< int32_t >(node.m_timeDuration.count());
                  if (ImGui::SliderInt("##distance", &seconds, 0, 10))
                  {
                     node.m_timeDuration = std::chrono::seconds(seconds);
                  }
               });
            }
         }
      }
   }

   ImGui::End();
}

void
EditorGUI::UpdateUI()
{
   ImGuiIO& io = ImGui::GetIO();
   io.DisplaySize = ImVec2(parent_.GetWindowSize().x, parent_.GetWindowSize().y);

   ImGui::NewFrame();

   windowSize_ = parent_.GetWindowSize();

   windowWidth_ = windowSize_.x / 7;
   toolsWindowHeight_ = windowSize_.y / 20;
   levelWindowHeight_ = windowSize_.y - toolsWindowHeight_;
   gameObjectWindowHeight_ = windowSize_.y;

   RenderMainPanel();

   if (currentLevel_)
   {
      RenderLevelMenu();
   }

   if (currentlySelectedGameObject_)
   {
      RenderGameObjectMenu();
   }

   if (exitPushed_)
   {
      RenderExitWindow();
   }

   ImGui::Render();
}

void
EditorGUI::GameObjectSelected(const std::shared_ptr< GameObject >& selectedGameObject)
{
   currentlySelectedGameObject_ = selectedGameObject;
}

void
EditorGUI::GameObjectUnselected()
{
   currentlySelectedGameObject_ = nullptr;
}

void
EditorGUI::EditorObjectSelected(const std::shared_ptr< EditorObject >& /*object*/)
{
   // m_currentlySelectedEditorObject = object;
}

void
EditorGUI::EditorObjectUnselected()
{
   // m_currentlySelectedEditorObject = nullptr;
}

void
EditorGUI::LevelLoaded(const std::shared_ptr< Level >& loadedLevel)
{
   currentLevel_ = loadedLevel;
}

void
EditorGUI::ObjectUpdated(Object::ID /*ID*/)
{
}

void
EditorGUI::ObjectDeleted(Object::ID /*ID*/)
{
}

} // namespace looper
