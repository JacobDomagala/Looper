#include "shader.hpp"
#include "logger/logger.hpp"
#include "renderer.hpp"
#include "texture.hpp"
#include "utils/assert.hpp"
#include "utils/file_manager.hpp"
#include "vulkan_common.hpp"

#include <algorithm>
#include <array>

namespace looper::renderer {
namespace {

VkShaderModule
CreateShaderModule(VkDevice device, const std::vector< char >& shaderByteCode)
{
   VkShaderModuleCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
   createInfo.codeSize = shaderByteCode.size();
   // NOLINTNEXTLINE
   createInfo.pCode = reinterpret_cast< const uint32_t* >(shaderByteCode.data());

   VkShaderModule shaderModule = {};
   vk_check_error(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule),
                  "Failed to create shader module!");

   return shaderModule;
}

} // namespace

std::pair< VertexShaderInfo, FragmentShaderInfo >
VulkanShader::CreateShader(VkDevice device, std::string_view vertex, std::string_view fragment)
{
   auto* vertShaderModule =
      CreateShaderModule(device, FileManager::ReadBinaryFile(SHADERS_DIR / vertex));
   auto* fragShaderModule =
      CreateShaderModule(device, FileManager::ReadBinaryFile(SHADERS_DIR / fragment));

   VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
   vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
   vertShaderStageInfo.module = vertShaderModule;
   vertShaderStageInfo.pName = "main";

   VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
   fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
   fragShaderStageInfo.module = fragShaderModule;
   fragShaderStageInfo.pName = "main";

   return {{device, vertShaderStageInfo}, {device, fragShaderStageInfo}};
}

///////////////////////////////////////////////////////////////////
///////////////////////    QUAD SHADER      ///////////////////////
///////////////////////////////////////////////////////////////////
void
QuadShader::CreateDescriptorPool()
{
   auto& renderData = Data::renderData_.at(GetCurrentlyBoundType());

   std::array< VkDescriptorPoolSize, 2 > poolSizes{};
   poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   poolSizes[0].descriptorCount = static_cast< uint32_t >(renderData.swapChainImages.size());
   poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   poolSizes[1].descriptorCount = static_cast< uint32_t >(renderData.swapChainImages.size());

   VkDescriptorPoolCreateInfo poolInfo = {};
   poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   poolInfo.poolSizeCount = static_cast< uint32_t >(poolSizes.size());
   poolInfo.pPoolSizes = poolSizes.data();
   poolInfo.maxSets = static_cast< uint32_t >(renderData.swapChainImages.size());

   vk_check_error(
      vkCreateDescriptorPool(Data::vk_device, &poolInfo, nullptr, &renderData.descriptorPool),
      "Failed to create descriptor pool!");
}

void
QuadShader::CreateDescriptorSetLayout()
{
   VkDescriptorSetLayoutBinding uboLayoutBinding = {};
   uboLayoutBinding.binding = 0;
   uboLayoutBinding.descriptorCount = 1;
   uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   uboLayoutBinding.pImmutableSamplers = nullptr;
   uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

   VkDescriptorSetLayoutBinding perInstanceBinding = {};
   perInstanceBinding.binding = 1;
   perInstanceBinding.descriptorCount = 1;
   perInstanceBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
   perInstanceBinding.pImmutableSamplers = nullptr;
   perInstanceBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

   VkDescriptorSetLayoutBinding texturesLayoutBinding = {};
   texturesLayoutBinding.binding = 2;
   texturesLayoutBinding.descriptorCount = MAX_NUM_TEXTURES;
   texturesLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   texturesLayoutBinding.pImmutableSamplers = nullptr;
   texturesLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

   auto bindings = std::to_array< VkDescriptorSetLayoutBinding >(
      {uboLayoutBinding, perInstanceBinding, texturesLayoutBinding});

   VkDescriptorSetLayoutCreateInfo layoutInfo = {};
   layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   layoutInfo.bindingCount = static_cast< uint32_t >(bindings.size());
   layoutInfo.pBindings = bindings.data();

   auto& renderData = Data::renderData_.at(GetCurrentlyBoundType());

   vk_check_error(vkCreateDescriptorSetLayout(Data::vk_device, &layoutInfo, nullptr,
                                              &renderData.descriptorSetLayout),
                  "Failed to create descriptor set layout!");
}

void
QuadShader::CreateDescriptorSets()
{
   auto& renderData = Data::renderData_.at(GetCurrentlyBoundType());
   const auto size = MAX_FRAMES_IN_FLIGHT;
   std::vector< VkDescriptorSetLayout > layouts(size, renderData.descriptorSetLayout);

   VkDescriptorSetAllocateInfo allocInfo = {};
   allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   allocInfo.descriptorPool = renderData.descriptorPool;
   allocInfo.descriptorSetCount = size;
   allocInfo.pSetLayouts = layouts.data();

   renderData.descriptorSets.resize(size);
   vk_check_error(
      vkAllocateDescriptorSets(Data::vk_device, &allocInfo, renderData.descriptorSets.data()),
      "Failed to allocate descriptor sets!");
}


void
QuadShader::UpdateDescriptorSets()
{
   auto& renderData = renderer::Data::renderData_.at(GetCurrentlyBoundType());

   // Explicit copy
   auto viewAndSamplers = TextureLibrary::GetViewSamplerPairs();

   // Fill all MAX_NUM_TEXTURES ImageViews
   for (size_t i = viewAndSamplers.size(); i < MAX_NUM_TEXTURES; ++i)
   {
      viewAndSamplers.push_back(viewAndSamplers.front());
   }

   std::vector< VkDescriptorImageInfo > descriptorImageInfos(viewAndSamplers.size(),
                                                             VkDescriptorImageInfo{});

   std::transform(descriptorImageInfos.begin(), descriptorImageInfos.end(), viewAndSamplers.begin(),
                  descriptorImageInfos.begin(),
                  [](auto& descriptoInfo, const auto& viewAndSampler) {
                     descriptoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                     descriptoInfo.imageView = viewAndSampler.first;
                     descriptoInfo.sampler = viewAndSampler.second;

                     return descriptoInfo;
                  });

   for (uint32_t frame = 0; frame < MAX_FRAMES_IN_FLIGHT; frame++)
   {
      WaitForFence(frame);

      VkDescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = renderData.uniformBuffers.at(frame).buffer_;
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(UniformBufferObject);


      VkDescriptorBufferInfo instanceBufferInfo = {};
      instanceBufferInfo.buffer = renderData.ssbo.at(frame).buffer_;
      instanceBufferInfo.offset = 0;
      instanceBufferInfo.range = renderData.ssbo.at(frame).bufferSize_;

      std::array< VkWriteDescriptorSet, 3 > descriptorWrites = {};

      descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[0].dstSet = renderData.descriptorSets.at(frame);
      descriptorWrites[0].dstBinding = 0;
      descriptorWrites[0].dstArrayElement = 0;
      descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrites[0].descriptorCount = 1;
      descriptorWrites[0].pBufferInfo = &bufferInfo;

      descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[1].dstSet = renderData.descriptorSets.at(frame);
      descriptorWrites[1].dstBinding = 1;
      descriptorWrites[1].dstArrayElement = 0;
      descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      descriptorWrites[1].descriptorCount = 1;
      descriptorWrites[1].pBufferInfo = &instanceBufferInfo;

      descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[2].dstSet = renderData.descriptorSets.at(frame);
      descriptorWrites[2].dstBinding = 2;
      descriptorWrites[2].dstArrayElement = 0;
      descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptorWrites[2].descriptorCount = static_cast< uint32_t >(viewAndSamplers.size());
      descriptorWrites[2].pImageInfo = descriptorImageInfos.data();

      vkUpdateDescriptorSets(Data::vk_device, static_cast< uint32_t >(descriptorWrites.size()),
                             descriptorWrites.data(), 0, nullptr);
   }
}

///////////////////////////////////////////////////////////////////
///////////////////////    LINE SHADER      ///////////////////////
///////////////////////////////////////////////////////////////////

void
LineShader::CreateDescriptorPool()
{
   auto& renderData = Data::renderData_.at(GetCurrentlyBoundType());
   std::array< VkDescriptorPoolSize, 1 > poolSizes{};
   poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   poolSizes[0].descriptorCount = static_cast< uint32_t >(renderData.swapChainImages.size());

   VkDescriptorPoolCreateInfo poolInfo = {};
   poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   poolInfo.poolSizeCount = static_cast< uint32_t >(poolSizes.size());
   poolInfo.pPoolSizes = poolSizes.data();
   poolInfo.maxSets = static_cast< uint32_t >(renderData.swapChainImages.size());

   vk_check_error(
      vkCreateDescriptorPool(Data::vk_device, &poolInfo, nullptr, &EditorData::lineDescriptorPool),
      "Failed to create line descriptor pool!");
}

void
LineShader::CreateDescriptorSetLayout()
{
   VkDescriptorSetLayoutBinding uboLayoutBinding = {};
   uboLayoutBinding.binding = 0;
   uboLayoutBinding.descriptorCount = 1;
   uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   uboLayoutBinding.pImmutableSamplers = nullptr;
   uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

   std::array< VkDescriptorSetLayoutBinding, 1 > bindings = {uboLayoutBinding};

   VkDescriptorSetLayoutCreateInfo lineLayoutInfo = {};
   lineLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   lineLayoutInfo.bindingCount = 1;
   lineLayoutInfo.pBindings = bindings.data();

   vk_check_error(vkCreateDescriptorSetLayout(Data::vk_device, &lineLayoutInfo, nullptr,
                                              &EditorData::lineDescriptorSetLayout_),
                  "Failed to create line descriptor set layout!");
}

void
LineShader::CreateDescriptorSets()
{
   const auto& renderData = Data::renderData_.at(GetCurrentlyBoundType());

   const auto size = MAX_FRAMES_IN_FLIGHT;
   std::vector< VkDescriptorSetLayout > layouts(size, EditorData::lineDescriptorSetLayout_);

   VkDescriptorSetAllocateInfo allocInfo = {};
   allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   allocInfo.descriptorPool = EditorData::lineDescriptorPool;
   allocInfo.descriptorSetCount = static_cast< uint32_t >(size);
   allocInfo.pSetLayouts = layouts.data();

   EditorData::lineDescriptorSets_.resize(size);
   vk_check_error(
      vkAllocateDescriptorSets(Data::vk_device, &allocInfo, EditorData::lineDescriptorSets_.data()),
      "Failed to allocate descriptor sets!");

   for (size_t i = 0; i < size; i++)
   {
      VkDescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = renderData.uniformBuffers[i].buffer_;
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(UniformBufferObject);

      std::array< VkWriteDescriptorSet, 1 > descriptorWrites = {};

      descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[0].dstSet = EditorData::lineDescriptorSets_[i];
      descriptorWrites[0].dstBinding = 0;
      descriptorWrites[0].dstArrayElement = 0;
      descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrites[0].descriptorCount = 1;
      descriptorWrites[0].pBufferInfo = &bufferInfo;

      vkUpdateDescriptorSets(Data::vk_device, static_cast< uint32_t >(descriptorWrites.size()),
                             descriptorWrites.data(), 0, nullptr);
   }
}

} // namespace looper::renderer
