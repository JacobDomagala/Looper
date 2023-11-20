#pragma once

#include "file_manager.hpp"
#include "texture.hpp"
#include "types.hpp"

#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

namespace looper::renderer {

struct TextureProperties
{
   VkFilter magFilter = VK_FILTER_LINEAR;
   VkFilter minFilter = VK_FILTER_LINEAR;

   VkSamplerAddressMode modeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
   VkSamplerAddressMode modeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
   VkSamplerAddressMode modeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
};

class Texture
{
 public:
   Texture(TextureType type, std::string_view textureName, TextureID id,
           const TextureProperties& props = {});
   Texture(TextureType type, std::string_view textureName, TextureID id,
           const FileManager::ImageData& data, const TextureProperties& props = {});

   Texture() = default;

   void
   Destroy();

   void
   UpdateTexture(const FileManager::ImageData& data) const;

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
   CreateSampler(uint32_t mipLevels = 1, const TextureProperties& props = {});

   static void
   TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
                         uint32_t mipLevels, bool cubemap = false);

   static void
   CopyBufferToImage(VkImage image, uint32_t texWidth, uint32_t texHeight, const uint8_t* data);

   static void
   CopyBufferToCubemapImage(VkImage image, uint32_t texWidth, uint32_t texHeight,
                            const uint8_t* data);

   static VkDescriptorSet
   CreateDescriptorSet(VkSampler sampler, VkImageView image_view, VkImageLayout image_layout,
                       VkDescriptorPool pool, VkDescriptorSetLayout layout);

   [[nodiscard]] std::pair< VkImageView, VkSampler >
   GetImageViewAndSampler() const;

   [[nodiscard]] VkImageView
   GetImageView() const;

   void
   CreateTextureSampler();

   [[nodiscard]] TextureType
   GetType() const;

   [[nodiscard]] const std::string&
   GetName() const;

   [[nodiscard]] VkImage
   GetImage() const;

   [[nodiscard]] TextureID
   GetID() const;

 private:
   void
   CreateTextureImage(const FileManager::ImageData& data);

 private:
   TextureID id_ = {};
   TextureType m_type = {};
   VkImage m_textureImage = {};
   VkDeviceMemory m_textureImageMemory = {};
   VkImageView m_textureImageView = {};
   VkSampler m_textureSampler = {};
   TextureProperties textureProps_ = {};
   VkFormat m_format = {};
   uint32_t m_mips = {};
   uint32_t m_width = {};
   uint32_t m_height = {};
   std::string m_name = "default_texture_name";
};

class TextureLibrary
{
 public:
   static const Texture*
   GetTexture(TextureType type, const std::string& textureName);

   static const Texture*
   GetTexture(const std::string& textureName);

   static Texture*
   GetTexture(const TextureID id);

   static const Texture*
   CreateTexture(TextureType type, const std::string& textureName,
                 const TextureProperties& props = {});

   static const Texture*
   CreateTexture(TextureType type, const std::string& textureName,
                 const FileManager::ImageData& data, const TextureProperties& props = {});

   static const std::vector< std::pair< VkImageView, VkSampler > >&
   GetViewSamplerPairs();

   [[nodiscard]] static uint32_t
   GetNumTextures();

   static void
   Clear();

 private:
   static void
   LoadTexture(TextureType type, std::string_view textureName, const TextureProperties& props = {});

   static void
   LoadTexture(TextureType type, std::string_view textureName, const FileManager::ImageData& data,
               const TextureProperties& props = {});

 private:
   static inline std::unordered_map< std::string, Texture > s_loadedTextures = {};
   static inline std::vector< std::pair< VkImageView, VkSampler > > viewSamplerPairs_ = {};
   static inline TextureID currentID_ = 0;
};

} // namespace looper::renderer
