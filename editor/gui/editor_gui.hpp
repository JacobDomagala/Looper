#pragma once

#include "object.hpp"
#include "renderer/buffer.hpp"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <memory>
#include <unordered_map>
#include <vector>

namespace looper {

class Editor;
class GameObject;
class EditorObject;
class GameObjectWindow;
class EditorObjectWindow;
class LevelWindow;
class Level;

struct PushConstBlock
{
   glm::vec2 scale = {};
   glm::vec2 translate = {};
};

class EditorGUI
{
 public:
   explicit EditorGUI(Editor& parent);

   void
   Init();

   static void
   Shutdown();

   void
   UpdateUI();

   void
   GameObjectSelected(const std::shared_ptr< GameObject >& selectedGameObject);

   void
   GameObjectUnselected();

   void
   EditorObjectSelected(const std::shared_ptr< EditorObject >& object);

   void
   EditorObjectUnselected();

   void
   LevelLoaded(const std::shared_ptr< Level >& loadedLevel);

   void
   ObjectUpdated(Object::ID ID);

   void
   ObjectDeleted(Object::ID ID);

   static bool
   IsBlockingEvents();

   static void
   UpdateBuffers();

   static void
   Render(VkCommandBuffer commandBuffer);

 private:
   void
   RenderMainPanel();
   void
   RenderLevelMenu();
   void
   RenderGameObjectMenu();

   void
   RenderCreateNewLevelWindow();

 private:
   static void
   PrepareResources();

   static void
   PreparePipeline(VkPipelineCache pipelineCache, VkRenderPass renderPass);

   Editor& m_parent;

   // EditorObjectWindow m_editorObjectWindow;
   std::shared_ptr< GameObject > m_currentlySelectedGameObject;
   std::shared_ptr< Level > m_currentLevel;

   glm::vec2 m_windowSize = {};
   float m_windowWidth = 0.0f;
   float m_toolsWindowHeight = 0.0f;
   float m_gameObjectWindowHeight = 0.0f;
   float m_levelWindowHeight = 0.0f;
   float m_debugWindowHeight = 0.0f;
   float m_debugWindowWidth = 0.0f;

   bool m_createPushed = false;

   inline static VkImage m_fontImage = {};
   inline static VkDeviceMemory m_fontMemory = {};
   inline static VkImageView m_fontView = {};
   inline static VkSampler m_sampler = {};
   inline static VkDescriptorPool m_descriptorPool = {};
   inline static VkDescriptorSetLayout m_descriptorSetLayout = {};
   inline static VkDescriptorSet m_descriptorSet = {};

   inline static VkPipeline m_pipeline = {};
   inline static VkPipelineLayout m_pipelineLayout = {};
   inline static uint32_t m_subpass = 0;

   inline static PushConstBlock m_pushConstant = {};
   inline static renderer::Buffer m_vertexBuffer = {};
   inline static renderer::Buffer m_indexBuffer = {};
   inline static int32_t m_vertexCount = 0;
   inline static int32_t m_indexCount = 0;
};

} // namespace looper
