#pragma once

#include "lighting.hpp"

#include <array>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace looper::renderer {

struct RenderData;

// Owned by one application/swapchain. Each frame slot owns all of its images;
// no pass reads lighting left over from a previous frame.
class RadianceCascades
{
 public:
   void
   Create(const RenderData& renderData);
   void
   Destroy(); // Caller must wait for the device before destroying resources.
   void
   Record(VkCommandBuffer commandBuffer, const RenderData& renderData, uint32_t frameIndex);
   [[nodiscard]] VkDescriptorImageInfo
   GetIrradiance(uint32_t frame) const;
   // Available for debug captures/readback after Record has completed.
   [[nodiscard]] VkImage
   GetIrradianceImage(uint32_t frame) const;

 private:
   struct Image
   {
      VkImage image = VK_NULL_HANDLE;
      VkImageView view = VK_NULL_HANDLE;
      VmaAllocation allocation = VK_NULL_HANDLE;
   };

   struct Frame
   {
      Image scene;
      std::array< Image, 2 > distance;
      std::array< Image, 2 > cascades;
      Image irradiance;
      VkFramebuffer framebuffer = VK_NULL_HANDLE;
      std::array< VkDescriptorSet, 2 > distanceSets = {};
      std::array< VkDescriptorSet, 2 > cascadeSets = {};
      VkDescriptorSet resolveSet = VK_NULL_HANDLE;
   };

   static Image
   CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);
   static void
   DestroyImage(Image& image);
   void
   CreateScenePipeline(const RenderData& renderData);
   VkPipeline
   CreateComputePipeline(const char* shader) const;
   VkDescriptorSet
   CreateSet(const Frame& frame, const Image& input, const Image& output,
             uint32_t distanceIndex) const;

   // Kept in sync with MAX_FRAMES_IN_FLIGHT (asserted in the implementation).
   std::array< Frame, 2 > frames_ = {};
   CascadeLayout layout_;
   uint32_t firstJump_ = 1;
   uint32_t finalDistance_ = 0;
   VkSampler sampler_ = VK_NULL_HANDLE;
   VkSampler linearSampler_ = VK_NULL_HANDLE;
   VkRenderPass scenePass_ = VK_NULL_HANDLE;
   VkPipelineLayout sceneLayout_ = VK_NULL_HANDLE;
   VkPipeline scenePipeline_ = VK_NULL_HANDLE;
   VkDescriptorSetLayout computeSetLayout_ = VK_NULL_HANDLE;
   VkDescriptorPool descriptorPool_ = VK_NULL_HANDLE;
   VkPipelineLayout computeLayout_ = VK_NULL_HANDLE;
   VkPipeline distancePipeline_ = VK_NULL_HANDLE;
   VkPipeline cascadePipeline_ = VK_NULL_HANDLE;
   VkPipeline resolvePipeline_ = VK_NULL_HANDLE;
};

} // namespace looper::renderer
