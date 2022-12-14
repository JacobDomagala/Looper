#pragma once

#include "texture.hpp"
#include "types.hpp"

#include <unordered_map>
#include <vulkan/vulkan.h>

namespace looper::render {

class Texture
{
 public:
   using TextureID = uint64_t;

   Texture(vulkan::TextureType type, std::string_view textureName, TextureID id);

   Texture() = default;

   void
   Destroy();

   void
   CreateTextureImage(vulkan::TextureType type, std::string_view textureName);

   static std::pair< VkImage, VkDeviceMemory >
   CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels,
               VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
               VkImageUsageFlags usage, VkMemoryPropertyFlags properties, bool cubemap = false);

   static void
   GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight,
                   uint32_t mipLevels);

   static VkImageView
   CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
                   uint32_t mipLevels, bool cubemap = false);

   static VkSampler
   CreateSampler(uint32_t mipLevels = 1);

   static void
   TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
                         uint32_t mipLevels, bool cubemap = false);

   static void
   CopyBufferToImage(VkImage image, uint32_t texWidth, uint32_t texHeight, uint8_t* data);

   static void
   CopyBufferToCubemapImage(VkImage image, uint32_t texWidth, uint32_t texHeight, uint8_t* data);

   [[nodiscard]] std::pair< VkImageView, VkSampler >
   GetImageViewAndSampler() const;

   void
   CreateTextureSampler();

   [[nodiscard]] vulkan::TextureType
   GetType() const;

   [[nodiscard]] const std::string&
   GetName() const;

   [[nodiscard]] VkImage
   GetImage() const;

   [[nodiscard]] TextureID
   GetID() const;

 private:
   void
   TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

   void
   CopyBufferToImage(uint8_t* data);

 private:
   TextureID id_ = {};
   vulkan::TextureType m_type = {};
   VkImage m_textureImage = {};
   VkDeviceMemory m_textureImageMemory = {};
   VkImageView m_textureImageView = {};
   VkSampler m_textureSampler = {};
   VkFormat m_format = {};
   // int32_t m_channels = {};
   uint32_t m_mips = {};
   uint32_t m_width = {};
   uint32_t m_height = {};
   std::string m_name = "default_texture_name";
};

class TextureLibrary
{
 public:
   static const Texture&
   GetTexture(vulkan::TextureType type, const std::string& textureName);

   static const Texture&
   GetTexture(const std::string& textureName);

   static const Texture&
   GetTexture(const Texture::TextureID id);

   static void
   CreateTexture(vulkan::TextureType type, const std::string& textureName);

   static void
   Clear();

 private:
   static void
   LoadTexture(vulkan::TextureType type, std::string_view textureName);

 private:
   static inline std::unordered_map< std::string, Texture > s_loadedTextures = {};
   static inline Texture::TextureID currentID_ = 0;
};

} // namespace shady::render
