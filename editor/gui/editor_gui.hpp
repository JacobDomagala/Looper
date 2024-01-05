#pragma once

#include "input/input_listener.hpp"
#include "object.hpp"
#include "renderer/buffer.hpp"
#include "time/time_step.hpp"

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

class EditorGUI : public InputListener
{
 public:
   explicit EditorGUI(Editor& parent);

   void
   KeyCallback(KeyEvent& event) override;

   void
   CharCallback(CharEvent& event) override;

   void
   MouseButtonCallback(MouseButtonEvent& event) override;

   void
   CursorPositionCallback(CursorPositionEvent& event) override;

   void
   MouseScrollCallback(MouseScrollEvent& event) override;

   void
   Init();

   static void
   Shutdown();

   void
   UpdateUI();

   void
   LevelLoaded(const std::shared_ptr< Level >& loadedLevel);

   void
   ObjectSelected(Object::ID ID);

   void
   ObjectUnselected(Object::ID ID);

   void
   ObjectUpdated(Object::ID ID);

   void
   ObjectDeleted(Object::ID ID);

   static bool
   IsBlockingEvents();

   void
   UpdateBuffers();

   void
   Render(VkCommandBuffer commandBuffer);

 private:
   void
   RenderMainPanel();

   void
   RenderLevelMenu();

   void
   RenderSelectedObjectsMenu();

   void
   RenderGameObjectMenu();

   void
   RenderGameObjectContent();

   void
   RenderCreateNewLevelWindow();

   void
   RenderExitWindow();

 private:
   static void
   PrepareResources();

   static void
   PreparePipeline();

   Editor& parent_;

   // EditorObjectWindow m_editorObjectWindow;
   Object::ID currentlySelectedGameObject_ = Object::INVALID_ID;
   std::shared_ptr< Level > currentLevel_ = {};
   time::TimeStep uiRenderTime = time::TimeStep{time::microseconds{}};

   glm::vec2 windowSize_ = {};
   float windowWidth_ = 0.0f;
   float toolsWindowHeight_ = 0.0f;
   float gameObjectWindowHeight_ = 0.0f;
   float levelWindowHeight_ = 0.0f;

   bool createPushed_ = false;
   bool exitPushed_ = false;

   // Data needed for loaded objects menu
   std::unordered_map< Object::ID, std::pair< std::string, bool > > objectsInfo_ = {};
   Object::ID setScrollTo_ = Object::INVALID_ID;

   inline static VkImage fontImage_ = {};
   inline static VkDeviceMemory fontMemory_ = {};
   inline static VkImageView m_fontView = {};
   inline static VkSampler sampler_ = {};
   inline static VkDescriptorPool descriptorPool_ = {};
   inline static VkDescriptorSetLayout descriptorSetLayout_ = {};
   inline static VkDescriptorSet descriptorSet_ = {};

   inline static VkPipeline pipeline_ = {};
   inline static VkPipelineLayout pipelineLayout_ = {};
   inline static uint32_t subpass_ = 0;

   inline static PushConstBlock pushConstant_ = {};
   inline static std::vector< renderer::Buffer > vertexBuffer_ = {};
   inline static std::vector< renderer::Buffer > indexBuffer_ = {};
   inline static std::vector< int32_t > vertexCount_ = {};
   inline static std::vector< int32_t > indexCount_ = {};
};

} // namespace looper
