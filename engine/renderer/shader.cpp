#include "shader.hpp"
#include "vulkan_common.hpp"
#include "renderer.hpp"
#include "texture.hpp"
#include "logger/logger.hpp"
#include "utils/assert.hpp"
#include "utils/file_manager.hpp"

#include <algorithm>

namespace looper::renderer {

static VkShaderModule
CreateShaderModule(VkDevice device, std::vector< char >&& shaderByteCode)
{
   VkShaderModuleCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
   createInfo.codeSize = shaderByteCode.size();
   //NOLINTNEXTLINE
   createInfo.pCode = reinterpret_cast< const uint32_t* >(shaderByteCode.data());

   VkShaderModule shaderModule = {};
   vk_check_error(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule),
            "Failed to create shader module!");

   return shaderModule;
}

std::pair< VertexShaderInfo, FragmentShaderInfo >
VulkanShader::CreateShader(VkDevice device, std::string_view vertex, std::string_view fragment)
{
   auto* vertShaderModule = CreateShaderModule(
      device, FileManager::ReadBinaryFile(SHADERS_DIR / vertex));
   auto* fragShaderModule = CreateShaderModule(
      device, FileManager::ReadBinaryFile(SHADERS_DIR / fragment));

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

void
QuadShader::CreateDescriptorPool()
{
   auto& renderData = Data::renderData_.at(VulkanRenderer::GetCurrentlyBoundType());

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

   VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
   samplerLayoutBinding.binding = 2;
   samplerLayoutBinding.descriptorCount = 1;
   samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
   samplerLayoutBinding.pImmutableSamplers = nullptr;
   samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

   VkDescriptorSetLayoutBinding texturesLayoutBinding = {};
   texturesLayoutBinding.binding = 3;
   texturesLayoutBinding.descriptorCount = MAX_NUM_TEXTURES;
   texturesLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   texturesLayoutBinding.pImmutableSamplers = nullptr;
   texturesLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

   std::array< VkDescriptorSetLayoutBinding, 4 > bindings = {
      uboLayoutBinding, perInstanceBinding, samplerLayoutBinding, texturesLayoutBinding};

   VkDescriptorSetLayoutCreateInfo layoutInfo = {};
   layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   layoutInfo.bindingCount = static_cast< uint32_t >(bindings.size());
   layoutInfo.pBindings = bindings.data();

   auto& renderData = Data::renderData_.at(VulkanRenderer::GetCurrentlyBoundType());

   vk_check_error(vkCreateDescriptorSetLayout(Data::vk_device, &layoutInfo, nullptr,
                                              &renderData.descriptorSetLayout),
                  "Failed to create descriptor set layout!");
}

void
QuadShader::CreateDescriptorSets()
{
   auto& renderData = Data::renderData_.at(VulkanRenderer::GetCurrentlyBoundType());
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
   vkDeviceWaitIdle(Data::vk_device);
   auto& renderData = renderer::Data::renderData_.at(VulkanRenderer::GetCurrentlyBoundType());

   const auto size = MAX_FRAMES_IN_FLIGHT;

   const auto [imageView, sampler] =
      TextureLibrary::GetTexture(TextureType::DIFFUSE_MAP, "white.png")->GetImageViewAndSampler();

   auto textureViews = TextureLibrary::GetTextureViews();

   // Fill all MAX_NUM_TEXTURES ImageViews
   for (size_t i = textureViews.size(); i < MAX_NUM_TEXTURES; ++i)
   {
      textureViews.push_back(textureViews.front());
   }

   std::vector< VkDescriptorImageInfo > descriptorImageInfos(textureViews.size(),
                                                             VkDescriptorImageInfo{});

   std::transform(descriptorImageInfos.begin(), descriptorImageInfos.end(), textureViews.begin(),
                  descriptorImageInfos.begin(),
                  [sampler = sampler](auto& descriptoInfo, const auto& texture) {
                     descriptoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                     descriptoInfo.sampler = sampler;
                     descriptoInfo.imageView = texture;
                     return descriptoInfo;
                  });

   for (size_t i = 0; i < size; i++)
   {
      VkDescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = renderData.uniformBuffers[i];
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(UniformBufferObject);


      VkDescriptorBufferInfo instanceBufferInfo = {};
      instanceBufferInfo.buffer = renderData.ssbo.at(i);
      instanceBufferInfo.offset = 0;
      instanceBufferInfo.range = renderData.perInstance.size() * sizeof(PerInstanceBuffer);

      VkDescriptorImageInfo imageInfo = {};
      imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imageInfo.imageView = imageView;
      imageInfo.sampler = sampler;

      std::array< VkWriteDescriptorSet, 4 > descriptorWrites = {};

      descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[0].dstSet = renderData.descriptorSets[i];
      descriptorWrites[0].dstBinding = 0;
      descriptorWrites[0].dstArrayElement = 0;
      descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrites[0].descriptorCount = 1;
      descriptorWrites[0].pBufferInfo = &bufferInfo;

      descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[1].dstSet = renderData.descriptorSets[i];
      descriptorWrites[1].dstBinding = 1;
      descriptorWrites[1].dstArrayElement = 0;
      descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      descriptorWrites[1].descriptorCount = 1;
      descriptorWrites[1].pBufferInfo = &instanceBufferInfo;

      VkDescriptorImageInfo samplerInfo = {};
      samplerInfo.sampler = sampler;

      descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[2].dstSet = renderData.descriptorSets[i];
      descriptorWrites[2].dstBinding = 2;
      descriptorWrites[2].dstArrayElement = 0;
      descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
      descriptorWrites[2].descriptorCount = 1;
      descriptorWrites[2].pImageInfo = &samplerInfo;

      descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrites[3].dstSet = renderData.descriptorSets[i];
      descriptorWrites[3].dstBinding = 3;
      descriptorWrites[3].dstArrayElement = 0;
      descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      descriptorWrites[3].descriptorCount = static_cast< uint32_t >(textureViews.size());
      descriptorWrites[3].pImageInfo = descriptorImageInfos.data();

      vkUpdateDescriptorSets(Data::vk_device, static_cast< uint32_t >(descriptorWrites.size()),
                             descriptorWrites.data(), 0, nullptr);
   }
}

void
LineShader::CreateDescriptorPool()
{
   auto& renderData = Data::renderData_.at(VulkanRenderer::GetCurrentlyBoundType());
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
   auto& renderData = Data::renderData_.at(VulkanRenderer::GetCurrentlyBoundType());

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
      bufferInfo.buffer = renderData.uniformBuffers[i];
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

} // namespace shady::renderer
