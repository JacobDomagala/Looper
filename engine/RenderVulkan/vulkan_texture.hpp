#pragma once

#include "vulkan_texture.hpp"
#include "types.hpp"

#include <unordered_map>
#include <vulkan/vulkan.h>

namespace looper::render::vulkan {
class Texture
{
 public:
   Texture(TextureType type, std::string_view textureName);

   Texture() = default;

   void
   Destroy();

   void
   CreateTextureImage(TextureType type, std::string_view textureName);

   static std::pair< VkImage, VkDeviceMemory >
   CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format,
               VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

   static void
   GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight,
                uint32_t mipLevels);

   static VkImageView
   CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

   std::pair<VkImageView, VkSampler>
   GetImageViewAndSampler() const;

   void
   CreateTextureSampler();

   TextureType
   GetType() const;

 private:
   void
   TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
   void
   CopyBufferToImage(VkBuffer buffer);

 private:
   TextureType m_type = {};
   VkImage m_textureImage = {};
   VkDeviceMemory m_textureImageMemory = {};
   VkImageView m_textureImageView = {};
   VkSampler m_textureSampler = {};
   VkFormat m_format = {};
   int32_t m_channels = {};
   uint32_t m_mips = {};
   uint32_t m_width = {};
   uint32_t m_height = {};
   std::string m_name = "default_texture_name";
};

class TextureLibrary
{
 public:
   static const Texture&
   GetTexture(TextureType type, const std::string& textureName);

   static const Texture&
   GetTexture(const std::string& textureName);

   static void 
   CreateTexture(TextureType type, const std::string& textureName);

   static void
   Clear();

 private:
   static void
   LoadTexture(TextureType type, std::string_view textureName);

 private:
   static inline std::unordered_map< std::string, Texture > s_loadedTextures = {};
};

} // namespace looper::render::vulkan
