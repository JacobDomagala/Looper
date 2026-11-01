#include "renderer/command.hpp"
#include "renderer/renderer.hpp"
#include "renderer/shader.hpp"
#include "renderer/texture.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/packing.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

using namespace looper::renderer;

namespace {
uint32_t validationErrors = 0;

VKAPI_ATTR VkBool32 VKAPI_CALL
ValidationCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT,
                   const VkDebugUtilsMessengerCallbackDataEXT* data, void*)
{
   if ((severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0)
   {
      ++validationErrors;
      std::cerr << data->pMessage << '\n';
   }
   return VK_FALSE;
}

void
Require(bool condition, const char* message)
{
   if (!condition)
   {
      std::cerr << message << '\n';
      std::exit(EXIT_FAILURE);
   }
}

void
Upload(const Buffer& buffer, const void* source, size_t bytes)
{
   void* mapped = nullptr;
   vk_check_error(vmaMapMemory(Data::vk_hAllocator, buffer.allocation_, &mapped),
                  "Map test buffer");
   memcpy(mapped, source, bytes);
   vmaUnmapMemory(Data::vk_hAllocator, buffer.allocation_);
}

Buffer
HostBuffer(size_t size, VkBufferUsageFlags usage)
{
   return Buffer::CreateBuffer(
      size, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

void
ImageBarrier(VkCommandBuffer command, VkImage image, VkImageLayout before, VkImageLayout after,
             VkAccessFlags source, VkAccessFlags destination, VkPipelineStageFlags sourceStage,
             VkPipelineStageFlags destinationStage)
{
   VkImageMemoryBarrier barrier = {};
   barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
   barrier.oldLayout = before;
   barrier.newLayout = after;
   barrier.srcAccessMask = source;
   barrier.dstAccessMask = destination;
   barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   barrier.image = image;
   barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
   vkCmdPipelineBarrier(command, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1,
                        &barrier);
}

void
CheckVisibleReceiver(const RenderData& data, const UniformBufferObject& uniform,
                     const std::vector< float >& irradiance, uint32_t frame)
{
   // Exercise the production sprite shaders as well as the compute output: an
   // incorrect UV crop or a widened main-camera projection must fail this test.
   const auto [width, height] = data.swapChainExtent;
   constexpr auto format = VK_FORMAT_R16G16B16A16_SFLOAT;
   const auto image =
      Texture::CreateImage(width, height, 1, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL,
                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
   const auto view =
      Texture::CreateImageView(image.textureImage_, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
   VkAttachmentDescription attachment = {};
   attachment.format = format;
   attachment.samples = VK_SAMPLE_COUNT_1_BIT;
   attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
   attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
   attachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
   const VkAttachmentReference reference = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
   VkSubpassDescription subpass = {};
   subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
   subpass.colorAttachmentCount = 1;
   subpass.pColorAttachments = &reference;
   const VkSubpassDependency dependency = {0,
                                           VK_SUBPASS_EXTERNAL,
                                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                                           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                           VK_ACCESS_TRANSFER_READ_BIT,
                                           0};
   VkRenderPassCreateInfo passInfo = {};
   passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
   passInfo.attachmentCount = 1;
   passInfo.pAttachments = &attachment;
   passInfo.subpassCount = 1;
   passInfo.pSubpasses = &subpass;
   passInfo.dependencyCount = 1;
   passInfo.pDependencies = &dependency;
   VkRenderPass pass = VK_NULL_HANDLE;
   vk_check_error(vkCreateRenderPass(Data::vk_device, &passInfo, nullptr, &pass),
                  "Create receiver render pass");
   VkFramebufferCreateInfo framebufferInfo = {};
   framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
   framebufferInfo.renderPass = pass;
   framebufferInfo.attachmentCount = 1;
   framebufferInfo.pAttachments = &view;
   framebufferInfo.width = width;
   framebufferInfo.height = height;
   framebufferInfo.layers = 1;
   VkFramebuffer framebuffer = VK_NULL_HANDLE;
   vk_check_error(vkCreateFramebuffer(Data::vk_device, &framebufferInfo, nullptr, &framebuffer),
                  "Create receiver framebuffer");
   VkPipelineLayoutCreateInfo layoutInfo = {};
   layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   layoutInfo.setLayoutCount = 1;
   layoutInfo.pSetLayouts = &data.descriptorSetLayout;
   VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
   vk_check_error(vkCreatePipelineLayout(Data::vk_device, &layoutInfo, nullptr, &pipelineLayout),
                  "Create receiver pipeline layout");
   auto [vertex, fragment] = VulkanShader::CreateShader(Data::vk_device, "vert.spv", "frag.spv");
   const auto stages = std::to_array({vertex.shaderInfo, fragment.shaderInfo});
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
   const VkViewport viewport = {0, 0, static_cast< float >(width), static_cast< float >(height),
                                0, 1};
   const VkRect2D scissor = {{0, 0}, {width, height}};
   VkPipelineViewportStateCreateInfo viewportState = {};
   viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
   viewportState.viewportCount = 1;
   viewportState.pViewports = &viewport;
   viewportState.scissorCount = 1;
   viewportState.pScissors = &scissor;
   VkPipelineRasterizationStateCreateInfo raster = {};
   raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
   raster.polygonMode = VK_POLYGON_MODE_FILL;
   raster.lineWidth = 1.0f;
   VkPipelineMultisampleStateCreateInfo samples = {};
   samples.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
   samples.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
   VkPipelineColorBlendAttachmentState blendAttachment = {};
   blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                    | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
   VkPipelineColorBlendStateCreateInfo blend = {};
   blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
   blend.attachmentCount = 1;
   blend.pAttachments = &blendAttachment;
   VkGraphicsPipelineCreateInfo pipelineInfo = {};
   pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
   pipelineInfo.stageCount = static_cast< uint32_t >(stages.size());
   pipelineInfo.pStages = stages.data();
   pipelineInfo.pVertexInputState = &vertexInput;
   pipelineInfo.pInputAssemblyState = &assembly;
   pipelineInfo.pViewportState = &viewportState;
   pipelineInfo.pRasterizationState = &raster;
   pipelineInfo.pMultisampleState = &samples;
   pipelineInfo.pColorBlendState = &blend;
   pipelineInfo.layout = pipelineLayout;
   pipelineInfo.renderPass = pass;
   VkPipeline pipeline = VK_NULL_HANDLE;
   vk_check_error(vkCreateGraphicsPipelines(Data::vk_device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                            nullptr, &pipeline),
                  "Create receiver pipeline");
   vertex.Destroy();
   fragment.Destroy();

   auto readback =
      HostBuffer(static_cast< size_t >(width) * height * 8, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
   const auto command = Command::BeginSingleTimeCommands();
   VkRenderPassBeginInfo begin = {};
   begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   begin.renderPass = pass;
   begin.framebuffer = framebuffer;
   begin.renderArea = scissor;
   const VkClearValue clear = {};
   begin.clearValueCount = 1;
   begin.pClearValues = &clear;
   vkCmdBeginRenderPass(command, &begin, VK_SUBPASS_CONTENTS_INLINE);
   vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
   vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
                           &data.descriptorSets.at(frame), 0, nullptr);
   const VkDeviceSize offset = 0;
   vkCmdBindVertexBuffers(command, 0, 1, &data.vertexBuffer[2].buffer_, &offset);
   vkCmdBindIndexBuffer(command, data.indexBuffer[2].buffer_, 0, VK_INDEX_TYPE_UINT32);
   vkCmdDrawIndexed(command, 6, 1, 12, 0, 0); // Draw only the white receiver.
   vkCmdEndRenderPass(command);
   VkBufferImageCopy copy = {};
   copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
   copy.imageExtent = {width, height, 1};
   vkCmdCopyImageToBuffer(command, image.textureImage_, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                          readback.buffer_, 1, &copy);
   Command::EndSingleTimeCommands(command);
   void* mapped = nullptr;
   vk_check_error(vmaMapMemory(Data::vk_hAllocator, readback.allocation_, &mapped),
                  "Read visible receiver");
   const auto* pixels = static_cast< const uint16_t* >(mapped);
   const auto layout = GetCascadeLayout(width, height);
   for (const auto fraction : {glm::vec2{0.05f, 0.05f}, glm::vec2{0.95f, 0.95f},
                               glm::vec2{0.2f, 0.5f}, glm::vec2{0.8f, 0.5f}})
   {
      const auto x = static_cast< uint32_t >(fraction.x * static_cast< float >(width));
      const auto y = static_cast< uint32_t >(fraction.y * static_cast< float >(height));
      const auto screenUV = (glm::vec2(x, y) + 0.5f) / glm::vec2(width, height);
      const auto lightPixel =
         ((screenUV - 0.5f) / uniform.lighting.w + 0.5f) * glm::vec2(layout.width, layout.height)
         - 0.5f;
      const glm::ivec2 lower = glm::floor(lightPixel);
      const auto weight = glm::fract(lightPixel);
      const auto sample = [&](int dx, int dy, size_t component) {
         const auto px = static_cast< uint32_t >(
            std::clamp(lower.x + dx, 0, static_cast< int >(layout.width) - 1));
         const auto py = static_cast< uint32_t >(
            std::clamp(lower.y + dy, 0, static_cast< int >(layout.height) - 1));
         return irradiance[(static_cast< size_t >(py) * layout.width + px) * 4 + component];
      };
      const auto index = (static_cast< size_t >(y) * width + x) * 4;
      for (size_t component = 0; component < 3; ++component)
      {
         const float expected = glm::mix(
            glm::mix(sample(0, 0, component), sample(1, 0, component), weight.x),
            glm::mix(sample(0, 1, component), sample(1, 1, component), weight.x), weight.y);
         Require(std::abs(glm::unpackHalf1x16(pixels[index + component]) - expected) < 0.01f,
                 "The sprite shader must sample the correct crop of expanded lighting");
      }
      Require(glm::unpackHalf1x16(pixels[index + 3]) == 1.0f,
              "Expanding lighting must not shrink visible sprites");
   }
   vmaUnmapMemory(Data::vk_hAllocator, readback.allocation_);
   readback.Destroy();
   vkDestroyPipeline(Data::vk_device, pipeline, nullptr);
   vkDestroyPipelineLayout(Data::vk_device, pipelineLayout, nullptr);
   vkDestroyFramebuffer(Data::vk_device, framebuffer, nullptr);
   vkDestroyRenderPass(Data::vk_device, pass, nullptr);
   vkDestroyImageView(Data::vk_device, view, nullptr);
   vmaDestroyImage(Data::vk_hAllocator, image.textureImage_, image.allocation_);
}

void
RunScene(uint32_t width, uint32_t height)
{
   auto storage = std::make_unique< RenderData >();
   auto& data = *storage;
   data.swapChainExtent = {width, height};
   data.lighting.enabled = true;
   data.lighting.offscreenMargin = 0.0f;
   std::array< VkDescriptorSetLayoutBinding, 4 > bindings = {
      {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
       {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
       {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_NUM_TEXTURES,
        VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
       {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}}};
   VkDescriptorSetLayoutCreateInfo layoutInfo = {};
   layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   layoutInfo.bindingCount = static_cast< uint32_t >(bindings.size());
   layoutInfo.pBindings = bindings.data();
   vk_check_error(
      vkCreateDescriptorSetLayout(Data::vk_device, &layoutInfo, nullptr, &data.descriptorSetLayout),
      "Create test layout");
   data.radianceCascades.Create(data);

   auto white = Texture::CreateImage(1, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM,
                                     VK_IMAGE_TILING_OPTIMAL,
                                     VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
   const auto whiteView = Texture::CreateImageView(white.textureImage_, VK_FORMAT_R8G8B8A8_UNORM,
                                                   VK_IMAGE_ASPECT_COLOR_BIT, 1);
   VkSamplerCreateInfo samplerInfo = {};
   samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
   VkSampler sampler = VK_NULL_HANDLE;
   vk_check_error(vkCreateSampler(Data::vk_device, &samplerInfo, nullptr, &sampler),
                  "Create sampler");
   auto command = Command::BeginSingleTimeCommands();
   ImageBarrier(command, white.textureImage_, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
   const VkClearColorValue whiteColor = {{1.0f, 1.0f, 1.0f, 1.0f}};
   const VkImageSubresourceRange range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
   vkCmdClearColorImage(command, white.textureImage_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        &whiteColor, 1, &range);
   ImageBarrier(command, white.textureImage_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT,
                VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
   Command::EndSingleTimeCommands(command);

   UniformBufferObject uniform = {};
   uniform.proj = glm::mat4(1.0f);
   uniform.view = glm::mat4(1.0f);
   uniform.lighting = {1.0f, 0.0f, 1.0f, GetLightingCoverageScale(data.lighting)};
   uniform.viewport = {static_cast< float >(width), static_cast< float >(height), 0.0f, 0.0f};
   auto uniformBuffer = HostBuffer(sizeof(uniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
   Upload(uniformBuffer, &uniform, sizeof(uniform));
   std::array< PerInstanceBuffer, 3 > instances = {};
   instances[0].model = glm::translate(glm::mat4(1.0f), {-0.6f, 0.0f, 0.0f})
                        * glm::scale(glm::mat4(1.0f), {0.2f, 0.8f, 1.0f});
   instances[0].color = glm::vec4(1.0f);
   instances[0].emissionOpacity = {8.0f, 2.0f, 0.0f, 1.0f};
   instances[1].model = glm::scale(glm::mat4(1.0f), {0.125f, 2.0f, 1.0f});
   instances[1].color = glm::vec4(1.0f);
   instances[1].emissionOpacity = {0.0f, 0.0f, 0.0f, 1.0f};
   instances[2].model = glm::scale(glm::mat4(1.0f), {2.0f, 2.0f, 1.0f});
   instances[2].color = glm::vec4(1.0f);
   instances[2].emissionOpacity = glm::vec4(0.0f); // Non-blocking white receiver.
   auto instanceBuffer = HostBuffer(sizeof(instances), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
   Upload(instanceBuffer, instances.data(), sizeof(instances));
   std::array< Vertex, 12 > vertices = {};
   constexpr std::array< glm::vec2, 4 > corners = {
      {{-0.5f, -0.5f}, {0.5f, -0.5f}, {0.5f, 0.5f}, {-0.5f, 0.5f}}};
   for (uint32_t i = 0; i < vertices.size(); ++i)
   {
      vertices[i].position_ = {corners[i % 4], 0.0f};
      vertices[i].texCoordsDraw_ = {0.5f, 0.5f, static_cast< float >(i / 4)};
   }
   constexpr std::array< uint32_t, 18 > indices = {0, 1, 2, 2, 3, 0,  4,  5,  6,
                                                   6, 7, 4, 8, 9, 10, 10, 11, 8};
   data.vertexBuffer[2] = HostBuffer(sizeof(vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
   data.indexBuffer[2] = HostBuffer(sizeof(indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
   Upload(data.vertexBuffer[2], vertices.data(), sizeof(vertices));
   Upload(data.indexBuffer[2], indices.data(), sizeof(indices));

   const std::array< VkDescriptorPoolSize, 3 > sizes = {
      {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2},
       {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2},
       {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2 * (MAX_NUM_TEXTURES + 1)}}};
   VkDescriptorPoolCreateInfo poolInfo = {};
   poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   poolInfo.maxSets = 2;
   poolInfo.poolSizeCount = static_cast< uint32_t >(sizes.size());
   poolInfo.pPoolSizes = sizes.data();
   vk_check_error(vkCreateDescriptorPool(Data::vk_device, &poolInfo, nullptr, &data.descriptorPool),
                  "Create test descriptor pool");
   std::array< VkDescriptorSetLayout, 2 > layouts = {data.descriptorSetLayout,
                                                     data.descriptorSetLayout};
   VkDescriptorSetAllocateInfo allocate = {};
   allocate.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   allocate.descriptorPool = data.descriptorPool;
   allocate.descriptorSetCount = 2;
   allocate.pSetLayouts = layouts.data();
   data.descriptorSets.resize(2);
   vk_check_error(vkAllocateDescriptorSets(Data::vk_device, &allocate, data.descriptorSets.data()),
                  "Allocate test descriptors");
   std::array< VkDescriptorImageInfo, MAX_NUM_TEXTURES > textures = {};
   textures.fill({sampler, whiteView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
   const VkDescriptorBufferInfo uniformInfo = {uniformBuffer.buffer_, 0, sizeof(uniform)};
   const VkDescriptorBufferInfo instanceInfo = {instanceBuffer.buffer_, 0, sizeof(instances)};
   for (uint32_t frame = 0; frame < 2; ++frame)
   {
      const auto irradiance = data.radianceCascades.GetIrradiance(frame);
      std::array< VkWriteDescriptorSet, 4 > writes = {};
      for (uint32_t binding = 0; binding < writes.size(); ++binding)
      {
         writes[binding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
         writes[binding].dstSet = data.descriptorSets[frame];
         writes[binding].dstBinding = binding;
         writes[binding].descriptorType = bindings[binding].descriptorType;
         writes[binding].descriptorCount = bindings[binding].descriptorCount;
      }
      writes[0].pBufferInfo = &uniformInfo;
      writes[1].pBufferInfo = &instanceInfo;
      writes[2].pImageInfo = textures.data();
      writes[3].pImageInfo = &irradiance;
      vkUpdateDescriptorSets(Data::vk_device, static_cast< uint32_t >(writes.size()), writes.data(),
                             0, nullptr);
   }

   const auto layout = GetCascadeLayout(width, height);
   const auto readbackSize =
      static_cast< size_t >(layout.width) * layout.height * 4 * sizeof(uint16_t);
   auto readback = HostBuffer(readbackSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
   const auto render = [&](uint32_t frame) {
      const auto cmd = Command::BeginSingleTimeCommands();
      data.radianceCascades.Record(cmd, data, frame);
      const auto output = data.radianceCascades.GetIrradianceImage(frame);
      ImageBarrier(cmd, output, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
      VkBufferImageCopy copy = {};
      copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
      copy.imageExtent = {layout.width, layout.height, 1};
      vkCmdCopyImageToBuffer(cmd, output, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readback.buffer_, 1,
                             &copy);
      ImageBarrier(cmd, output, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
                   VK_ACCESS_TRANSFER_READ_BIT,
                   VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                   VK_PIPELINE_STAGE_TRANSFER_BIT,
                   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
      Command::EndSingleTimeCommands(cmd);
      void* mapped = nullptr;
      vk_check_error(vmaMapMemory(Data::vk_hAllocator, readback.allocation_, &mapped),
                     "Read irradiance");
      const auto* halves = static_cast< const uint16_t* >(mapped);
      std::vector< float > pixels(readbackSize / sizeof(uint16_t));
      std::transform(halves, halves + pixels.size(), pixels.begin(),
                     [](uint16_t value) { return glm::unpackHalf1x16(value); });
      vmaUnmapMemory(Data::vk_hAllocator, readback.allocation_);
      for (const auto value : pixels)
      {
         Require(std::isfinite(value) && value >= 0.0f,
                 "Irradiance must be finite and nonnegative");
      }
      return pixels;
   };

   data.numMeshes[2] = 1;
   const auto lit = render(0);
   const auto sample =
      (static_cast< size_t >(layout.height / 2) * layout.width + layout.width * 4 / 5) * 4;
   Require(lit[sample] > 0.01f && lit[sample + 1] > 0.0f && lit[sample + 2] == 0.0f,
           "An emissive sprite must propagate its color into empty space");
   data.numMeshes[2] = 2;
   const auto shadow = render(1);
   std::cout << width << 'x' << height << ": lit=" << lit[sample]
             << ", behind wall=" << shadow[sample] << '\n';
   Require(shadow[sample] < lit[sample] * 0.15f, "An opaque wall must block propagated light");
   instances[1].emissionOpacity.w = -1.0f; // Unlit, zero blocking opacity.
   Upload(instanceBuffer, instances.data(), sizeof(instances));
   const auto nonblocking = render(1);
   Require(std::abs(nonblocking[sample] - lit[sample]) < 0.001f,
           "An unlit, non-blocking sprite must not enter the lighting field");
   instances[1].emissionOpacity.w = -2.0f; // Unlit, fully blocking opacity.
   Upload(instanceBuffer, instances.data(), sizeof(instances));
   const auto unlitWall = render(1);
   Require(unlitWall[sample] < lit[sample] * 0.15f,
           "Blocking must work independently of receiving light");
   // Reuse a frame after hiding the emitter: no stale radiance or transparent-sprite occlusion.
   instances[0].color.a = 0.0f;
   Upload(instanceBuffer, instances.data(), sizeof(instances));
   const auto dark = render(0);
   for (size_t i = 0; i < dark.size(); i += 4)
   {
      Require(dark[i] == 0.0f && dark[i + 1] == 0.0f && dark[i + 2] == 0.0f,
              "Hidden emitters must leave no residual light in a reused frame");
   }
   data.numMeshes[2] = 0;
   const auto empty = render(1);
   Require(empty[sample] == 0.0f, "An empty scene must resolve to black");
   data.lighting.enabled = false;
   command = Command::BeginSingleTimeCommands();
   data.radianceCascades.Record(command, data, 0);
   Command::EndSingleTimeCommands(command);
   data.lighting.enabled = true;
   instances[0].color.a = 1.0f;
   Upload(instanceBuffer, instances.data(), sizeof(instances));
   data.numMeshes[2] = 1;
   const auto reenabled = render(0);
   Require(std::abs(reenabled[sample] - lit[sample]) < 0.001f,
           "Re-enabling lighting must regenerate the current scene");
   instances[0].model[3].x = -2.0f;
   Upload(instanceBuffer, instances.data(), sizeof(instances));
   const auto offscreen = render(0);
   for (size_t i = 0; i < offscreen.size(); i += 4)
   {
      Require(offscreen[i] == 0.0f && offscreen[i + 1] == 0.0f && offscreen[i + 2] == 0.0f,
              "Zero margin must retain viewport-only capture without clamped edge light");
   }

   data.lighting.offscreenMargin = 0.5f;
   uniform.lighting.w = GetLightingCoverageScale(data.lighting);
   Upload(uniformBuffer, &uniform, sizeof(uniform));
   const auto center =
      (static_cast< size_t >(layout.height / 2) * layout.width + layout.width / 2) * 4;
   for (const auto direction :
        {glm::vec2{-1, 0}, glm::vec2{1, 0}, glm::vec2{0, -1}, glm::vec2{0, 1}})
   {
      const auto rotation =
         glm::rotate(glm::mat4(1.0f), std::atan2(direction.y, direction.x), {0.0f, 0.0f, 1.0f});
      instances[0].model = glm::translate(glm::mat4(1.0f), glm::vec3(direction * 1.35f, 0.0f))
                           * rotation * glm::scale(glm::mat4(1.0f), {0.25f, 0.8f, 1.0f});
      instances[1].model = glm::translate(glm::mat4(1.0f), glm::vec3(direction * 1.1f, 0.0f))
                           * rotation * glm::scale(glm::mat4(1.0f), {0.15f, 4.0f, 1.0f});
      instances[1].emissionOpacity.w = 1.0f;
      Upload(instanceBuffer, instances.data(), sizeof(instances));
      data.numMeshes[2] = 1;
      const auto expanded = render(0);
      Require(expanded[center] > 0.01f && expanded[center + 1] > 0.0f,
              "A fully off-screen emitter must illuminate the visible viewport on every edge");
      CheckVisibleReceiver(data, uniform, expanded, 0);
      data.numMeshes[2] = 2;
      const auto blocked = render(1);
      Require(blocked[center] < expanded[center] * 0.15f,
              "Off-screen walls must block off-screen emitters");
   }

   // Coverage changes must take effect without recreating resources or retaining old light.
   data.numMeshes[2] = 1;
   instances[0].model = glm::translate(glm::mat4(1.0f), {-3.0f, 0.0f, 0.0f})
                        * glm::scale(glm::mat4(1.0f), {0.3f, 1.0f, 1.0f});
   Upload(instanceBuffer, instances.data(), sizeof(instances));
   const auto beyondMargin = render(0);
   Require(beyondMargin[center] == 0.0f,
           "Emitters beyond the capture margin must leave no stale light");
   data.lighting.offscreenMargin = 2.0f;
   uniform.lighting.w = GetLightingCoverageScale(data.lighting);
   Upload(uniformBuffer, &uniform, sizeof(uniform));
   const auto extended = render(0);
   Require(extended[center] > 0.01f, "Increasing the margin must include more distant emitters");
   CheckVisibleReceiver(data, uniform, extended, 0);
   data.lighting.offscreenMargin = 0.0f;
   uniform.lighting.w = GetLightingCoverageScale(data.lighting);
   Upload(uniformBuffer, &uniform, sizeof(uniform));
   const auto reduced = render(0);
   Require(reduced[center] == 0.0f,
           "Reducing the margin must remove out-of-range light immediately");

   // A fixed world-space emitter crosses a moving/zooming camera's edge.
   data.lighting.offscreenMargin = 0.5f;
   uniform.lighting.w = GetLightingCoverageScale(data.lighting);
   instances[0].model = glm::translate(glm::mat4(1.0f), {-0.6f, 0.0f, 0.0f})
                        * glm::scale(glm::mat4(1.0f), {0.2f, 0.8f, 1.0f});
   for (const float cameraX : {0.0f, 0.75f, 1.0f})
   {
      uniform.view = glm::translate(glm::mat4(1.0f), {-cameraX, 0.1f, 0.0f});
      uniform.proj = glm::scale(glm::mat4(1.0f), {1.1f, -1.1f, 1.0f});
      instances[2].model = glm::inverse(uniform.proj * uniform.view)
                           * glm::scale(glm::mat4(1.0f), {2.0f, 2.0f, 1.0f});
      Upload(uniformBuffer, &uniform, sizeof(uniform));
      Upload(instanceBuffer, instances.data(), sizeof(instances));
      const auto moved = render(1);
      Require(moved[center] > 0.01f,
              "Camera movement must retain light after the emitter leaves view");
      CheckVisibleReceiver(data, uniform, moved, 1);
   }
   std::cout << "Off-screen emitters, blockers, coverage changes, camera movement, and visible "
                "shading passed\n";

   readback.Destroy();
   vkDestroyDescriptorPool(Data::vk_device, data.descriptorPool, nullptr);
   data.radianceCascades.Destroy();
   vkDestroyDescriptorSetLayout(Data::vk_device, data.descriptorSetLayout, nullptr);
   data.vertexBuffer[2].Destroy();
   data.indexBuffer[2].Destroy();
   uniformBuffer.Destroy();
   instanceBuffer.Destroy();
   vkDestroySampler(Data::vk_device, sampler, nullptr);
   vkDestroyImageView(Data::vk_device, whiteView, nullptr);
   vmaDestroyImage(Data::vk_hAllocator, white.textureImage_, white.allocation_);
}
} // namespace

int
main()
{
   VkApplicationInfo application = {};
   application.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
   application.apiVersion = VK_API_VERSION_1_2;
   uint32_t count = 0;
   vkEnumerateInstanceLayerProperties(&count, nullptr);
   std::vector< VkLayerProperties > layers(count);
   vkEnumerateInstanceLayerProperties(&count, layers.data());
   const char* validationLayer = "VK_LAYER_KHRONOS_validation";
   const bool validation = std::any_of(layers.begin(), layers.end(), [&](const auto& layer) {
      return strcmp(layer.layerName, validationLayer) == 0;
   });
   const char* debugExtension = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
   VkInstanceCreateInfo instanceInfo = {};
   instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   instanceInfo.pApplicationInfo = &application;
   instanceInfo.enabledLayerCount = validation ? 1u : 0u;
   instanceInfo.ppEnabledLayerNames = &validationLayer;
   instanceInfo.enabledExtensionCount = validation ? 1u : 0u;
   instanceInfo.ppEnabledExtensionNames = &debugExtension;
   VkValidationFeatureEnableEXT sync = VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT;
   VkValidationFeaturesEXT validationFeatures = {};
   validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
   validationFeatures.enabledValidationFeatureCount = 1;
   validationFeatures.pEnabledValidationFeatures = &sync;
   instanceInfo.pNext = validation ? &validationFeatures : nullptr;
   if (vkCreateInstance(&instanceInfo, nullptr, &Data::vk_instance) != VK_SUCCESS)
   {
      std::cout << "SKIP: No Vulkan runtime\n";
      return 77;
   }
   VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
   if (validation)
   {
      VkDebugUtilsMessengerCreateInfoEXT info = {};
      info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                         | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
      info.pfnUserCallback = ValidationCallback;
      const auto createMessenger = reinterpret_cast< PFN_vkCreateDebugUtilsMessengerEXT >(
         vkGetInstanceProcAddr(Data::vk_instance, "vkCreateDebugUtilsMessengerEXT"));
      vk_check_error(createMessenger(Data::vk_instance, &info, nullptr, &messenger),
                     "Debug messenger");
   }
   vkEnumeratePhysicalDevices(Data::vk_instance, &count, nullptr);
   if (count == 0)
   {
      std::cout << "SKIP: No Vulkan device\n";
      return 77;
   }
   std::vector< VkPhysicalDevice > devices(count);
   vkEnumeratePhysicalDevices(Data::vk_instance, &count, devices.data());
   Data::vk_physicalDevice = devices.front();
   vkGetPhysicalDeviceQueueFamilyProperties(Data::vk_physicalDevice, &count, nullptr);
   std::vector< VkQueueFamilyProperties > queues(count);
   vkGetPhysicalDeviceQueueFamilyProperties(Data::vk_physicalDevice, &count, queues.data());
   uint32_t family = 0;
   while (family < count
          && (queues[family].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
                != (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
   {
      ++family;
   }
   Require(family < count, "No graphics/compute queue");
   const float priority = 1.0f;
   VkDeviceQueueCreateInfo queueInfo = {};
   queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
   queueInfo.queueFamilyIndex = family;
   queueInfo.queueCount = 1;
   queueInfo.pQueuePriorities = &priority;
   VkDeviceCreateInfo deviceInfo = {};
   deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
   deviceInfo.queueCreateInfoCount = 1;
   deviceInfo.pQueueCreateInfos = &queueInfo;
   vk_check_error(vkCreateDevice(Data::vk_physicalDevice, &deviceInfo, nullptr, &Data::vk_device),
                  "Create test device");
   vkGetDeviceQueue(Data::vk_device, family, 0, &Data::vk_graphicsQueue);
   VmaAllocatorCreateInfo allocatorInfo = {};
   allocatorInfo.instance = Data::vk_instance;
   allocatorInfo.physicalDevice = Data::vk_physicalDevice;
   allocatorInfo.device = Data::vk_device;
   allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
   vk_check_error(vmaCreateAllocator(&allocatorInfo, &Data::vk_hAllocator),
                  "Create test allocator");
   VkCommandPoolCreateInfo poolInfo = {};
   poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   poolInfo.queueFamilyIndex = family;
   poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
   vk_check_error(vkCreateCommandPool(Data::vk_device, &poolInfo, nullptr, &Data::commandPool),
                  "Create test command pool");
   RunScene(128, 96);
   RunScene(131, 99);
   RunScene(1920, 1080);
   vmaDestroyAllocator(Data::vk_hAllocator);
   vkDestroyCommandPool(Data::vk_device, Data::commandPool, nullptr);
   vkDestroyDevice(Data::vk_device, nullptr);
   if (messenger != VK_NULL_HANDLE)
   {
      const auto destroyMessenger = reinterpret_cast< PFN_vkDestroyDebugUtilsMessengerEXT >(
         vkGetInstanceProcAddr(Data::vk_instance, "vkDestroyDebugUtilsMessengerEXT"));
      destroyMessenger(Data::vk_instance, messenger, nullptr);
   }
   vkDestroyInstance(Data::vk_instance, nullptr);
   Require(validationErrors == 0, "Vulkan validation errors occurred");
   std::cout << "Radiance cascades GPU tests passed (validation "
             << (validation ? "enabled" : "unavailable") << ")\n";
}
