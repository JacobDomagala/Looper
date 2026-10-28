#include "radiance_cascades.hpp"

#include "command.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "utils/file_manager.hpp"
#include "vulkan_common.hpp"

#include <algorithm>
#include <vector>

namespace looper::renderer {
namespace {
constexpr VkFormat RADIANCE_FORMAT = VK_FORMAT_R16G16B16A16_SFLOAT;
constexpr VkFormat DISTANCE_FORMAT = VK_FORMAT_R32G32B32A32_SFLOAT;

struct CascadePushConstants
{
   glm::ivec2 extent;
   int32_t cascade;
   int32_t cascadeCount;
   int32_t jump;
};
static_assert(sizeof(CascadePushConstants) == 20);
static_assert(MAX_FRAMES_IN_FLIGHT == 2);

void
ComputeBarrier(VkCommandBuffer commandBuffer,
               VkPipelineStageFlags destination = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT)
{
   VkMemoryBarrier barrier = {};
   barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
   barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
   barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
   vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, destination, 0, 1,
                        &barrier, 0, nullptr, 0, nullptr);
}
} // namespace

RadianceCascades::Image
RadianceCascades::CreateImage(uint32_t width, uint32_t height, VkFormat format,
                              VkImageUsageFlags usage)
{
   Image result;
   VkImageCreateInfo info = {};
   info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
   info.imageType = VK_IMAGE_TYPE_2D;
   info.format = format;
   info.extent = {width, height, 1};
   info.mipLevels = 1;
   info.arrayLayers = 1;
   info.samples = VK_SAMPLE_COUNT_1_BIT;
   info.tiling = VK_IMAGE_TILING_OPTIMAL;
   info.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;
   info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
   VmaAllocationCreateInfo allocationInfo = {};
   allocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
   vk_check_error(vmaCreateImage(Data::vk_hAllocator, &info, &allocationInfo, &result.image,
                                 &result.allocation, nullptr),
                  "Failed to create a radiance cascades image");
   result.view = Texture::CreateImageView(result.image, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
   return result;
}

void
RadianceCascades::DestroyImage(Image& image)
{
   vkDestroyImageView(Data::vk_device, image.view, nullptr);
   if (image.image != VK_NULL_HANDLE)
   {
      vmaDestroyImage(Data::vk_hAllocator, image.image, image.allocation);
   }
   image = {};
}

void
RadianceCascades::CreateScenePipeline(const RenderData& renderData)
{
   VkAttachmentDescription attachment = {};
   attachment.format = RADIANCE_FORMAT;
   attachment.samples = VK_SAMPLE_COUNT_1_BIT;
   attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
   attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
   attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
   attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
   attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   VkAttachmentReference colorReference = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
   VkSubpassDescription subpass = {};
   subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
   subpass.colorAttachmentCount = 1;
   subpass.pColorAttachments = &colorReference;
   std::array< VkSubpassDependency, 2 > dependencies = {};
   dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
   dependencies[0].dstSubpass = 0;
   dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
   dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
   dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
   dependencies[1].srcSubpass = 0;
   dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
   dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
   dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
   dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
   VkRenderPassCreateInfo passInfo = {};
   passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
   passInfo.attachmentCount = 1;
   passInfo.pAttachments = &attachment;
   passInfo.subpassCount = 1;
   passInfo.pSubpasses = &subpass;
   passInfo.dependencyCount = static_cast< uint32_t >(dependencies.size());
   passInfo.pDependencies = dependencies.data();
   vk_check_error(vkCreateRenderPass(Data::vk_device, &passInfo, nullptr, &scenePass_),
                  "Failed to create lighting scene render pass");

   VkPipelineLayoutCreateInfo layoutInfo = {};
   layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   layoutInfo.setLayoutCount = 1;
   layoutInfo.pSetLayouts = &renderData.descriptorSetLayout;
   vk_check_error(vkCreatePipelineLayout(Data::vk_device, &layoutInfo, nullptr, &sceneLayout_),
                  "Failed to create lighting scene pipeline layout");

   auto [vertexShader, fragmentShader] =
      VulkanShader::CreateShader(Data::vk_device, "vert.spv", "radiance_scene.frag.spv");
   const auto stages = std::to_array({vertexShader.shaderInfo, fragmentShader.shaderInfo});
   const auto binding = Vertex::getBindingDescription();
   const auto attributes = Vertex::getAttributeDescriptions();
   VkPipelineVertexInputStateCreateInfo vertexInput = {};
   vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
   vertexInput.vertexBindingDescriptionCount = 1;
   vertexInput.pVertexBindingDescriptions = &binding;
   vertexInput.vertexAttributeDescriptionCount = static_cast< uint32_t >(attributes.size());
   vertexInput.pVertexAttributeDescriptions = attributes.data();
   VkPipelineInputAssemblyStateCreateInfo assembly = {};
   assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
   assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
   VkPipelineViewportStateCreateInfo viewport = {};
   viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
   viewport.viewportCount = 1;
   viewport.scissorCount = 1;
   const auto dynamicStates = std::to_array({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR});
   VkPipelineDynamicStateCreateInfo dynamic = {};
   dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
   dynamic.dynamicStateCount = static_cast< uint32_t >(dynamicStates.size());
   dynamic.pDynamicStates = dynamicStates.data();
   VkPipelineRasterizationStateCreateInfo raster = {};
   raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
   raster.polygonMode = VK_POLYGON_MODE_FILL;
   raster.cullMode = VK_CULL_MODE_NONE;
   raster.lineWidth = 1.0f;
   VkPipelineMultisampleStateCreateInfo samples = {};
   samples.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
   samples.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
   VkPipelineColorBlendAttachmentState blend = {};
   blend.blendEnable = VK_TRUE;
   blend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
   blend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
   blend.colorBlendOp = VK_BLEND_OP_ADD;
   blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
   blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
   blend.alphaBlendOp = VK_BLEND_OP_ADD;
   blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                          | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
   VkPipelineColorBlendStateCreateInfo blending = {};
   blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
   blending.attachmentCount = 1;
   blending.pAttachments = &blend;
   VkGraphicsPipelineCreateInfo pipelineInfo = {};
   pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
   pipelineInfo.stageCount = static_cast< uint32_t >(stages.size());
   pipelineInfo.pStages = stages.data();
   pipelineInfo.pVertexInputState = &vertexInput;
   pipelineInfo.pInputAssemblyState = &assembly;
   pipelineInfo.pViewportState = &viewport;
   pipelineInfo.pRasterizationState = &raster;
   pipelineInfo.pMultisampleState = &samples;
   pipelineInfo.pColorBlendState = &blending;
   pipelineInfo.pDynamicState = &dynamic;
   pipelineInfo.layout = sceneLayout_;
   pipelineInfo.renderPass = scenePass_;
   vk_check_error(vkCreateGraphicsPipelines(Data::vk_device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                            nullptr, &scenePipeline_),
                  "Failed to create lighting scene pipeline");
   vertexShader.Destroy();
   fragmentShader.Destroy();
}

VkPipeline
RadianceCascades::CreateComputePipeline(const char* shader) const
{
   const auto bytes = FileManager::ReadBinaryFile(SHADERS_DIR / shader);
   // Copy to aligned storage: vector<char> does not guarantee uint32_t alignment.
   utils::Assert(!bytes.empty() && bytes.size() % sizeof(uint32_t) == 0,
                 "Invalid radiance cascades shader");
   std::vector< uint32_t > words(bytes.size() / sizeof(uint32_t));
   memcpy(words.data(), bytes.data(), bytes.size());
   VkShaderModuleCreateInfo moduleInfo = {};
   moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
   moduleInfo.codeSize = bytes.size();
   moduleInfo.pCode = words.data();
   VkShaderModule module = VK_NULL_HANDLE;
   vk_check_error(vkCreateShaderModule(Data::vk_device, &moduleInfo, nullptr, &module),
                  "Failed to load radiance cascades shader");
   VkComputePipelineCreateInfo info = {};
   info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
   info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
   info.stage.module = module;
   info.stage.pName = "main";
   info.layout = computeLayout_;
   VkPipeline result = VK_NULL_HANDLE;
   const auto status =
      vkCreateComputePipelines(Data::vk_device, VK_NULL_HANDLE, 1, &info, nullptr, &result);
   vkDestroyShaderModule(Data::vk_device, module, nullptr);
   vk_check_error(status, "Failed to create radiance cascades compute pipeline");
   return result;
}

VkDescriptorSet
RadianceCascades::CreateSet(const Frame& frame, const Image& input, const Image& output,
                            uint32_t distanceIndex) const
{
   VkDescriptorSetAllocateInfo allocation = {};
   allocation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   allocation.descriptorPool = descriptorPool_;
   allocation.descriptorSetCount = 1;
   allocation.pSetLayouts = &computeSetLayout_;
   VkDescriptorSet result = VK_NULL_HANDLE;
   vk_check_error(vkAllocateDescriptorSets(Data::vk_device, &allocation, &result),
                  "Failed to allocate radiance cascades descriptors");
   const std::array< VkDescriptorImageInfo, 4 > images = {
      {{sampler_, frame.scene.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
       {sampler_, input.view, VK_IMAGE_LAYOUT_GENERAL},
       {VK_NULL_HANDLE, output.view, VK_IMAGE_LAYOUT_GENERAL},
       {sampler_, frame.distance.at(distanceIndex).view, VK_IMAGE_LAYOUT_GENERAL}}};
   std::array< VkWriteDescriptorSet, 4 > writes = {};
   for (uint32_t i = 0; i < writes.size(); ++i)
   {
      writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      writes[i].dstSet = result;
      writes[i].dstBinding = i;
      writes[i].descriptorCount = 1;
      writes[i].descriptorType =
         i == 2 ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      writes[i].pImageInfo = &images[i];
   }
   vkUpdateDescriptorSets(Data::vk_device, static_cast< uint32_t >(writes.size()), writes.data(), 0,
                          nullptr);
   return result;
}

void
RadianceCascades::Create(const RenderData& renderData)
{
   layout_ = GetCascadeLayout(renderData.swapChainExtent.width, renderData.swapChainExtent.height);
   for (const auto format : {RADIANCE_FORMAT, DISTANCE_FORMAT})
   {
      VkFormatProperties properties = {};
      vkGetPhysicalDeviceFormatProperties(Data::vk_physicalDevice, format, &properties);
      VkFormatFeatureFlags required =
         VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
      if (format == RADIANCE_FORMAT)
      {
         required |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT
                     | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
      }
      utils::Assert((properties.optimalTilingFeatures & required) == required,
                    "GPU does not support radiance cascades image formats");
   }
   firstJump_ = 1;
   while (firstJump_ < std::max(layout_.width, layout_.height))
   {
      firstJump_ *= 2;
   }
   firstJump_ /= 2;
   finalDistance_ = 0;
   for (auto jump = firstJump_; jump > 0; jump /= 2)
   {
      finalDistance_ = 1u - finalDistance_;
   }
   // An additional unit jump (JFA+1) reduces distance-field errors near corners.
   finalDistance_ = 1u - finalDistance_;

   VkSamplerCreateInfo samplerInfo = {};
   samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
   samplerInfo.magFilter = VK_FILTER_NEAREST;
   samplerInfo.minFilter = VK_FILTER_NEAREST;
   samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
   samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
   samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
   samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
   vk_check_error(vkCreateSampler(Data::vk_device, &samplerInfo, nullptr, &sampler_),
                  "Failed to create lighting sampler");
   samplerInfo.magFilter = VK_FILTER_LINEAR;
   samplerInfo.minFilter = VK_FILTER_LINEAR;
   vk_check_error(vkCreateSampler(Data::vk_device, &samplerInfo, nullptr, &linearSampler_),
                  "Failed to create irradiance sampler");
   CreateScenePipeline(renderData);

   std::array< VkDescriptorSetLayoutBinding, 4 > bindings = {};
   for (uint32_t i = 0; i < bindings.size(); ++i)
   {
      bindings[i].binding = i;
      bindings[i].descriptorCount = 1;
      bindings[i].descriptorType =
         i == 2 ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
   }
   VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
   descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   descriptorLayoutInfo.bindingCount = static_cast< uint32_t >(bindings.size());
   descriptorLayoutInfo.pBindings = bindings.data();
   vk_check_error(vkCreateDescriptorSetLayout(Data::vk_device, &descriptorLayoutInfo, nullptr,
                                              &computeSetLayout_),
                  "Failed to create radiance cascades descriptor layout");
   const VkPushConstantRange pushRange = {VK_SHADER_STAGE_COMPUTE_BIT, 0,
                                          sizeof(CascadePushConstants)};
   VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
   pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   pipelineLayoutInfo.setLayoutCount = 1;
   pipelineLayoutInfo.pSetLayouts = &computeSetLayout_;
   pipelineLayoutInfo.pushConstantRangeCount = 1;
   pipelineLayoutInfo.pPushConstantRanges = &pushRange;
   vk_check_error(
      vkCreatePipelineLayout(Data::vk_device, &pipelineLayoutInfo, nullptr, &computeLayout_),
      "Failed to create radiance cascades pipeline layout");
   distancePipeline_ = CreateComputePipeline("radiance_distance.comp.spv");
   cascadePipeline_ = CreateComputePipeline("radiance_cascade.comp.spv");
   resolvePipeline_ = CreateComputePipeline("radiance_resolve.comp.spv");
   constexpr uint32_t setCount = MAX_FRAMES_IN_FLIGHT * 5;
   const std::array< VkDescriptorPoolSize, 2 > sizes = {
      {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, setCount * 3},
       {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, setCount}}};
   VkDescriptorPoolCreateInfo poolInfo = {};
   poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   poolInfo.maxSets = setCount;
   poolInfo.poolSizeCount = static_cast< uint32_t >(sizes.size());
   poolInfo.pPoolSizes = sizes.data();
   vk_check_error(vkCreateDescriptorPool(Data::vk_device, &poolInfo, nullptr, &descriptorPool_),
                  "Failed to create radiance cascades descriptor pool");

   const auto commandBuffer = Command::BeginSingleTimeCommands();
   for (auto& frame : frames_)
   {
      frame.scene = CreateImage(layout_.width, layout_.height, RADIANCE_FORMAT,
                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
      for (auto& image : frame.distance)
      {
         image =
            CreateImage(layout_.width, layout_.height, DISTANCE_FORMAT, VK_IMAGE_USAGE_STORAGE_BIT);
      }
      for (auto& image : frame.cascades)
      {
         image = CreateImage(layout_.atlasWidth, layout_.atlasHeight, RADIANCE_FORMAT,
                             VK_IMAGE_USAGE_STORAGE_BIT);
      }
      frame.irradiance = CreateImage(layout_.width, layout_.height, RADIANCE_FORMAT,
                                     VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
      for (const auto* image : {&frame.distance[0], &frame.distance[1], &frame.cascades[0],
                                &frame.cascades[1], &frame.irradiance})
      {
         VkImageMemoryBarrier barrier = {};
         barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
         barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
         barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
         barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
         barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
         barrier.image = image->image;
         barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
         barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
         vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                              VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
                                 | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                              0, 0, nullptr, 0, nullptr, 1, &barrier);
      }
      VkFramebufferCreateInfo framebufferInfo = {};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = scenePass_;
      framebufferInfo.attachmentCount = 1;
      framebufferInfo.pAttachments = &frame.scene.view;
      framebufferInfo.width = layout_.width;
      framebufferInfo.height = layout_.height;
      framebufferInfo.layers = 1;
      vk_check_error(
         vkCreateFramebuffer(Data::vk_device, &framebufferInfo, nullptr, &frame.framebuffer),
         "Failed to create lighting framebuffer");
      for (uint32_t i = 0; i < 2; ++i)
      {
         frame.distanceSets[i] =
            CreateSet(frame, frame.distance[1u - i], frame.distance[i], finalDistance_);
         frame.cascadeSets[i] =
            CreateSet(frame, frame.cascades[1u - i], frame.cascades[i], finalDistance_);
      }
      // Cascade zero always writes atlas zero, independently of cascade count.
      frame.resolveSet = CreateSet(frame, frame.cascades[0], frame.irradiance, finalDistance_);
   }
   Command::EndSingleTimeCommands(commandBuffer);
}

void
RadianceCascades::Record(VkCommandBuffer commandBuffer, const RenderData& renderData,
                         uint32_t frameIndex)
{
   if (!renderData.lighting.enabled)
   {
      return;
   }
   const auto& frame = frames_.at(frameIndex);
   const VkClearValue clear = {};
   VkRenderPassBeginInfo begin = {};
   begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   begin.renderPass = scenePass_;
   begin.framebuffer = frame.framebuffer;
   begin.renderArea.extent = {layout_.width, layout_.height};
   begin.clearValueCount = 1;
   begin.pClearValues = &clear;
   vkCmdBeginRenderPass(commandBuffer, &begin, VK_SUBPASS_CONTENTS_INLINE);
   const VkViewport viewport = {
      0.0f, 0.0f, static_cast< float >(layout_.width), static_cast< float >(layout_.height),
      0.0f, 1.0f};
   const VkRect2D scissor = {{0, 0}, {layout_.width, layout_.height}};
   vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
   vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
   vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, scenePipeline_);
   vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sceneLayout_, 0, 1,
                           &renderData.descriptorSets.at(frameIndex), 0, nullptr);
   const VkDeviceSize offset = 0;
   for (uint32_t layer = NUM_LAYERS; layer-- > 0;)
   {
      if (renderData.numMeshes[layer] == 0)
      {
         continue;
      }
      vkCmdBindVertexBuffers(commandBuffer, 0, 1, &renderData.vertexBuffer[layer].buffer_, &offset);
      vkCmdBindIndexBuffer(commandBuffer, renderData.indexBuffer[layer].buffer_, 0,
                           VK_INDEX_TYPE_UINT32);
      vkCmdDrawIndexed(commandBuffer, renderData.numMeshes[layer] * INDICES_PER_SPRITE, 1, 0, 0, 0);
   }
   vkCmdEndRenderPass(commandBuffer);

   CascadePushConstants push = {
      {static_cast< int32_t >(layout_.width), static_cast< int32_t >(layout_.height)},
      0,
      static_cast< int32_t >(layout_.count),
      0};
   const auto dispatch = [&](VkPipeline pipeline, VkDescriptorSet set, uint32_t width,
                             uint32_t height) {
      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
      vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computeLayout_, 0, 1,
                              &set, 0, nullptr);
      vkCmdPushConstants(commandBuffer, computeLayout_, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                         sizeof(push), &push);
      vkCmdDispatch(commandBuffer, (width + 7u) / 8u, (height + 7u) / 8u, 1);
      ComputeBarrier(commandBuffer);
   };
   dispatch(distancePipeline_, frame.distanceSets[0], layout_.width, layout_.height);
   uint32_t output = 1;
   for (auto jump = firstJump_; jump > 0; jump /= 2)
   {
      push.jump = static_cast< int32_t >(jump);
      dispatch(distancePipeline_, frame.distanceSets[output], layout_.width, layout_.height);
      output = 1u - output;
   }
   push.jump = 1;
   dispatch(distancePipeline_, frame.distanceSets[output], layout_.width, layout_.height);
   for (uint32_t cascade = layout_.count; cascade-- > 0;)
   {
      push.cascade = static_cast< int32_t >(cascade);
      const auto spacing = 2u << cascade;
      const auto width = ((layout_.width + spacing - 1u) / spacing) * spacing;
      const auto height = ((layout_.height + spacing - 1u) / spacing) * spacing;
      dispatch(cascadePipeline_, frame.cascadeSets[cascade % 2u], width, height);
   }
   dispatch(resolvePipeline_, frame.resolveSet, layout_.width, layout_.height);
   ComputeBarrier(commandBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}

VkDescriptorImageInfo
RadianceCascades::GetIrradiance(uint32_t frame) const
{
   return {linearSampler_, frames_.at(frame).irradiance.view, VK_IMAGE_LAYOUT_GENERAL};
}

VkImage
RadianceCascades::GetIrradianceImage(uint32_t frame) const
{
   return frames_.at(frame).irradiance.image;
}

void
RadianceCascades::Destroy()
{
   vkDestroyDescriptorPool(Data::vk_device, descriptorPool_, nullptr);
   for (auto& frame : frames_)
   {
      vkDestroyFramebuffer(Data::vk_device, frame.framebuffer, nullptr);
      DestroyImage(frame.scene);
      DestroyImage(frame.irradiance);
      for (auto& image : frame.distance)
      {
         DestroyImage(image);
      }
      for (auto& image : frame.cascades)
      {
         DestroyImage(image);
      }
   }
   for (const auto pipeline :
        {scenePipeline_, distancePipeline_, cascadePipeline_, resolvePipeline_})
   {
      vkDestroyPipeline(Data::vk_device, pipeline, nullptr);
   }
   vkDestroyPipelineLayout(Data::vk_device, sceneLayout_, nullptr);
   vkDestroyPipelineLayout(Data::vk_device, computeLayout_, nullptr);
   vkDestroyDescriptorSetLayout(Data::vk_device, computeSetLayout_, nullptr);
   vkDestroyRenderPass(Data::vk_device, scenePass_, nullptr);
   vkDestroySampler(Data::vk_device, sampler_, nullptr);
   vkDestroySampler(Data::vk_device, linearSampler_, nullptr);
   *this = {};
}

} // namespace looper::renderer
