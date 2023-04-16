#include "texture.hpp"
#include "buffer.hpp"
#include "command.hpp"
#include "logger/logger.hpp"
#include "utils/assert.hpp"
#include "utils/file_manager.hpp"
#include "vulkan_common.hpp"
#include "renderer.hpp"

#undef max

#include <string_view>
#include <ranges>

namespace looper::renderer {

/**************************************************************************************************
 ****************************************** TEXTURE ***********************************************
 *************************************************************************************************/

void
Texture::Destroy()
{
   vkDestroySampler(Data::vk_device, m_textureSampler, nullptr);
   vkDestroyImageView(Data::vk_device, m_textureImageView, nullptr);
   vkDestroyImage(Data::vk_device, m_textureImage, nullptr);
   vkFreeMemory(Data::vk_device, m_textureImageMemory, nullptr);
}

Texture::Texture(TextureType type, std::string_view textureName, TextureID id)
   : id_(id), m_name(std::string(textureName))
{
   CreateTextureImage(type, textureName);
}

void
Texture::CreateTextureImage(TextureType type, std::string_view textureName)
{
   m_type = type;
   auto textureData = FileManager::LoadImageData(textureName);
   m_width = static_cast< uint32_t >(textureData.m_size.x);
   m_height = static_cast< uint32_t >(textureData.m_size.y);
   m_format = type == TextureType::DIFFUSE_MAP ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
   m_mips = static_cast< uint32_t >(std::floor(std::log2(std::max(m_width, m_height)))) + 1;

   std::tie(m_textureImage, m_textureImageMemory) = CreateImage(
      m_width, m_height, m_mips, VK_SAMPLE_COUNT_1_BIT, m_format, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
         | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);

   m_textureImageView =
      CreateImageView(m_textureImage, m_format, VK_IMAGE_ASPECT_COLOR_BIT, m_mips, false);

   CreateTextureSampler();

   TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_mips);

   CopyBufferToImage(textureData.m_bytes.get());

   // transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
   GenerateMipmaps(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, static_cast< int32_t >(m_width),
                   static_cast< int32_t >(m_height), m_mips);
}

std::pair< VkImage, VkDeviceMemory >
Texture::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels,
                     VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
                     VkImageUsageFlags usage, VkMemoryPropertyFlags properties, bool cubemap)
{
   VkImage image = {};
   VkDeviceMemory imageMemory = {};

   VkImageCreateInfo imageInfo = {};
   imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
   imageInfo.imageType = VK_IMAGE_TYPE_2D;
   imageInfo.extent.width = width;
   imageInfo.extent.height = height;
   imageInfo.extent.depth = 1;
   imageInfo.mipLevels = mipLevels;
   imageInfo.arrayLayers = 1;
   imageInfo.format = format;
   imageInfo.tiling = tiling;
   imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   imageInfo.usage = usage;
   imageInfo.samples = numSamples;
   imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
   if (cubemap)
   {
      // Cube faces count as array layers in Vulkan
      imageInfo.arrayLayers = 6;
      // This flag is required for cube map images
      imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
   }

   vk_check_error(vkCreateImage(Data::vk_device, &imageInfo, nullptr, &image),
                  "failed to create image!");

   Buffer::AllocateImageMemory(image, imageMemory, properties);

   vkBindImageMemory(Data::vk_device, image, imageMemory, 0);

   return {image, imageMemory};
}

VkImageView
Texture::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
                         uint32_t mipLevels, bool /*cubemap*/)
{
   VkImageViewCreateInfo viewInfo = {};
   viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
   viewInfo.image = image;
   viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
   viewInfo.format = format;
   viewInfo.subresourceRange.baseMipLevel = 0;
   viewInfo.subresourceRange.levelCount = mipLevels;
   viewInfo.subresourceRange.aspectMask = aspectFlags;
   viewInfo.subresourceRange.baseArrayLayer = 0;
   viewInfo.subresourceRange.layerCount = 1;

   VkImageView imageView = {};
   vk_check_error(vkCreateImageView(Data::vk_device, &viewInfo, nullptr, &imageView),
                  "Failed to create texture image view!");

   return imageView;
}

VkSampler
Texture::CreateSampler(uint32_t mipLevels)
{
   VkSampler sampler = {};

   VkPhysicalDeviceProperties properties = {};
   vkGetPhysicalDeviceProperties(Data::vk_physicalDevice, &properties);

   VkSamplerCreateInfo samplerInfo = {};
   samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
   samplerInfo.magFilter = VK_FILTER_LINEAR;
   samplerInfo.minFilter = VK_FILTER_LINEAR;
   samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
   samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
   samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
   samplerInfo.anisotropyEnable = VK_TRUE;
   samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
   samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
   samplerInfo.unnormalizedCoordinates = VK_FALSE;
   samplerInfo.compareEnable = VK_FALSE;
   samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
   samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
   samplerInfo.minLod = 0.0f;
   samplerInfo.maxLod = static_cast< float >(mipLevels);
   samplerInfo.mipLodBias = 0.0f;

   vk_check_error(vkCreateSampler(Data::vk_device, &samplerInfo, nullptr, &sampler),
                  "Failed to create texture sampler!");

   return sampler;
}

void
Texture::GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight,
                         uint32_t mipLevels)
{
   // Check if image format supports linear blitting
   VkFormatProperties formatProperties = {};
   vkGetPhysicalDeviceFormatProperties(Data::vk_physicalDevice, imageFormat, &formatProperties);

   if (!(formatProperties.optimalTilingFeatures
         & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
   {
      utils::Assert(false, "Texture image format does not support linear blitting!");
   }

   VkCommandBuffer commandBuffer = Command::BeginSingleTimeCommands();

   VkImageMemoryBarrier barrier = {};
   barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
   barrier.image = image;
   barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
   barrier.subresourceRange.baseArrayLayer = 0;
   barrier.subresourceRange.layerCount = 1;
   barrier.subresourceRange.levelCount = 1;

   auto mipWidth = texWidth;
   auto mipHeight = texHeight;

   for (uint32_t i = 1; i < mipLevels; i++)
   {
      barrier.subresourceRange.baseMipLevel = i - 1;
      barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

      vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

      VkImageBlit blit = {};
      blit.srcOffsets[0] = {0, 0, 0};
      blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
      blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      blit.srcSubresource.mipLevel = i - 1;
      blit.srcSubresource.baseArrayLayer = 0;
      blit.srcSubresource.layerCount = 1;
      blit.dstOffsets[0] = {0, 0, 0};
      blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
      blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      blit.dstSubresource.mipLevel = i;
      blit.dstSubresource.baseArrayLayer = 0;
      blit.dstSubresource.layerCount = 1;

      vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

      barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                           &barrier);

      if (mipWidth > 1)
      {
         mipWidth /= 2;
      }

      if (mipHeight > 1)
      {
         mipHeight /= 2;
      }
   }

   barrier.subresourceRange.baseMipLevel = mipLevels - 1;
   barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
   barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
   barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

   vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                        &barrier);

   Command::EndSingleTimeCommands(commandBuffer);
}

std::pair< VkImageView, VkSampler >
Texture::GetImageViewAndSampler() const
{
   return {m_textureImageView, m_textureSampler};
}

VkImageView
Texture::GetImageView() const
{
   return m_textureImageView;
}

TextureType
Texture::GetType() const
{
   return m_type;
}

const std::string&
Texture::GetName() const
{
   return m_name;
}

VkImage
Texture::GetImage() const
{
   return m_textureImage;
}

TextureID
Texture::GetID() const
{
   return id_;
}

void
Texture::CreateTextureSampler()
{
   m_textureSampler = CreateSampler(m_mips);
}

void
Texture::CopyBufferToImage(VkImage image, uint32_t texWidth, uint32_t texHeight, uint8_t* data)
{
   VkBufferImageCopy region = {};
   region.bufferOffset = 0;
   region.bufferRowLength = 0;
   region.bufferImageHeight = 0;
   region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
   region.imageSubresource.mipLevel = 0;
   region.imageSubresource.baseArrayLayer = 0;
   region.imageSubresource.layerCount = 1;
   region.imageOffset = {0, 0, 0};
   region.imageExtent = {texWidth, texHeight, 1};

   const auto size =
      static_cast< size_t >(texWidth) * static_cast< size_t >(texHeight) * static_cast< size_t >(4);
   Buffer::CopyDataToImageWithStaging(image, data, size, {region});
}

void
Texture::CopyBufferToCubemapImage(VkImage image, uint32_t texWidth, uint32_t texHeight,
                                  uint8_t* data)
{
   constexpr auto num_faces = 6;
   const auto single_face_size =
      static_cast< size_t >(4) * static_cast< size_t >(texWidth) * static_cast< size_t >(texHeight);
   const auto total_buffer_size = num_faces * single_face_size;

   // Setup buffer copy regions for each face
   std::vector< VkBufferImageCopy > bufferCopyRegions;

   for (uint32_t face = 0; face < 6; ++face)
   {
      VkBufferImageCopy bufferCopyRegion = {};
      bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      bufferCopyRegion.imageSubresource.mipLevel = 0;
      bufferCopyRegion.imageSubresource.baseArrayLayer = face;
      bufferCopyRegion.imageSubresource.layerCount = 1;
      bufferCopyRegion.imageExtent.width = texWidth;
      bufferCopyRegion.imageExtent.height = texHeight;
      bufferCopyRegion.imageExtent.depth = 1;
      bufferCopyRegion.bufferOffset = face * single_face_size;

      bufferCopyRegions.push_back(bufferCopyRegion);
   }

   Texture::TransitionImageLayout(image, VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, true);

   Buffer::CopyDataToImageWithStaging(image, data, total_buffer_size, bufferCopyRegions);

   Texture::TransitionImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, true);
}

void
Texture::CopyBufferToImage(uint8_t* data)
{
   CopyBufferToImage(m_textureImage, m_width, m_height, data);
}

void
Texture::TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
                               uint32_t mipLevels, bool cubemap)
{
   VkCommandBuffer commandBuffer = Command::BeginSingleTimeCommands();

   VkImageMemoryBarrier barrier = {};
   barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
   barrier.oldLayout = oldLayout;
   barrier.newLayout = newLayout;
   barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   barrier.image = image;
   barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
   barrier.subresourceRange.baseMipLevel = 0;
   barrier.subresourceRange.levelCount = mipLevels;
   barrier.subresourceRange.baseArrayLayer = 0;
   barrier.subresourceRange.layerCount = not cubemap ? 1 : 6;

   VkPipelineStageFlags sourceStage = {};
   VkPipelineStageFlags destinationStage = {};

   if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
   {
      barrier.srcAccessMask = 0;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

      sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
   }
   else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
   {
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
      destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
   }
   else
   {
      utils::Assert(false, "Unsupported layout transition!");
   }

   vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1,
                        &barrier);

   Command::EndSingleTimeCommands(commandBuffer);
}

void
Texture::TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
{
   TransitionImageLayout(m_textureImage, oldLayout, newLayout, mipLevels);
}


/**************************************************************************************************
 *************************************** TEXTURE LIBRARY ******************************************
 *************************************************************************************************/
const Texture*
TextureLibrary::GetTexture(TextureType type, const std::string& textureName)
{
   if (s_loadedTextures.find(textureName) == s_loadedTextures.end())
   {
      Logger::Debug("Texture: {} not found in library. Loading it", textureName);
      LoadTexture(type, textureName);
   }

   return &s_loadedTextures[textureName];
}

const Texture*
TextureLibrary::GetTexture(const std::string& textureName)
{
   return GetTexture(TextureType::DIFFUSE_MAP, textureName);
}

const Texture*
TextureLibrary::GetTexture(const TextureID id)
{
   for (auto& texture : s_loadedTextures)
   {
      if (texture.second.GetID() == id)
      {
         return &texture.second;
      }
   }

   utils::Assert(true, fmt::format("TextureLibrary::GetTexture (With TextureID = {}) failed! "
                                   "Requested texture ID is not present!\n",
                                   id));

   return nullptr;
}

void
TextureLibrary::CreateTexture(TextureType type, const std::string& textureName)
{
   if (s_loadedTextures.find(textureName) == s_loadedTextures.end())
   {
      Logger::Debug("Creating texture {}", textureName);
      LoadTexture(type, textureName);
   }
   else
   {
      Logger::Debug("Texture {} already loaded!", textureName);
   }
}

void
TextureLibrary::Clear()
{
   s_loadedTextures.clear();
}

void
TextureLibrary::LoadTexture(TextureType type, std::string_view textureName)
{
   s_loadedTextures[std::string{textureName}] = {type, textureName, currentID_++};
   imageViews_.push_back(s_loadedTextures[std::string{textureName}].GetImageView());

   VulkanRenderer::UpdateDescriptors();
}

const std::vector< VkImageView >&
TextureLibrary::GetTextureViews()
{
   return imageViews_;
}

uint32_t
TextureLibrary::GetNumTextures()
{
   return static_cast<uint32_t>(currentID_);
}

} // namespace looper::renderer
