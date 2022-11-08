#include "vulkan_shader.hpp"
#include "vulkan_common.hpp"
#include "Logger/logger.hpp"
#include "utils/assert.hpp"
#include "FileManager/FileManager.hpp"

namespace looper::render::vulkan {

static VkShaderModule
CreateShaderModule(VkDevice device, std::vector< char >&& shaderByteCode)
{
   VkShaderModuleCreateInfo createInfo{};
   createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
   createInfo.codeSize = shaderByteCode.size();
   createInfo.pCode = reinterpret_cast< const uint32_t* >(shaderByteCode.data());

   VkShaderModule shaderModule;
   VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule),
            "Failed to create shader module!");
  
   return shaderModule;
}

std::pair< VertexShaderInfo, FragmentShaderInfo >
VulkanShader::CreateShader(VkDevice device, std::string_view vertex, std::string_view fragment)
{
   VkShaderModule vertShaderModule = CreateShaderModule(
      device, FileManager::ReadBinaryFile(SHADERS_DIR / vertex));
   VkShaderModule fragShaderModule = CreateShaderModule(
      device, FileManager::ReadBinaryFile(SHADERS_DIR / fragment));

   VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
   vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
   vertShaderStageInfo.module = vertShaderModule;
   vertShaderStageInfo.pName = "main";

   VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
   fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
   fragShaderStageInfo.module = fragShaderModule;
   fragShaderStageInfo.pName = "main";

   return {{device, vertShaderStageInfo}, {device, fragShaderStageInfo}};
}

} // namespace looper::render::vulkan
