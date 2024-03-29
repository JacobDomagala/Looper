#include "editor_gui.hpp"
#include "animatable.hpp"
#include "editor.hpp"
#include "game_object.hpp"
#include "helpers.hpp"
#include "icons.hpp"
#include "input/input_manager.hpp"
#include "renderer/renderer.hpp"
#include "renderer/shader.hpp"
#include "renderer/texture.hpp"
#include "renderer/types.hpp"
#include "renderer/vulkan_common.hpp"
#include "types.hpp"
#include "utils/file_manager.hpp"

#include <GLFW/glfw3.h>
#include <fmt/format.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <cstdint>


namespace looper {

EditorGUI::EditorGUI(Editor& parent) : parent_(parent)
{
}

void
EditorGUI::RecalculateCommonRenderLayerAndColision()
{
   if (selectedObjects_.empty())
   {
      return;
   }

   const auto& [idFirst, collisionFirst, layerFirst] = selectedObjects_.front();
   commonRenderLayer_ = {true, layerFirst};
   commonCollision_ = {true, collisionFirst};

   for (uint32_t idx = 1; idx < selectedObjects_.size(); idx++)
   {
      const auto& [id, collision, layer] = selectedObjects_.at(idx);
      if (commonRenderLayer_.first and (layer != commonRenderLayer_.second))
      {
         commonRenderLayer_.first = false;
      }
      if (commonCollision_.first and (collision != commonCollision_.second))
      {
         commonCollision_.first = false;
      }
   }
}

void
EditorGUI::KeyCallback(KeyEvent& event)
{
   // This HAS to be here as Editor should handle key callbacks first
   parent_.KeyCallback(event);

   if (not event.handled_)
   {
      ImGuiIO& io = ImGui::GetIO();

      const auto imguiKey = KeyToImGuiKey(event.key_);
      io.AddKeyEvent(imguiKey, (event.action_ == GLFW_PRESS));

      if ((event.key_ == GLFW_KEY_ESCAPE) and (event.action_ == GLFW_PRESS))
      {
         exitPushed_ = not exitPushed_;
         event.handled_ = true;
      }
   }
}

void
EditorGUI::CharCallback(CharEvent& event)
{
   ImGuiIO& io = ImGui::GetIO();
   io.AddInputCharacter(event.key_);
}

void
EditorGUI::MouseButtonCallback(MouseButtonEvent& event)
{
   auto& io = ImGui::GetIO();

   io.MouseDown[0] = (event.button_ == GLFW_MOUSE_BUTTON_1) && event.action_;
   io.MouseDown[1] = (event.button_ == GLFW_MOUSE_BUTTON_2) && event.action_;
}

void
EditorGUI::CursorPositionCallback(CursorPositionEvent& event)
{
   auto& io = ImGui::GetIO();
   io.MousePos = ImVec2(static_cast< float >(event.xPos_), static_cast< float >(event.yPos_));
}

void
EditorGUI::MouseScrollCallback(MouseScrollEvent& event)
{
   auto& io = ImGui::GetIO();
   io.MouseWheelH += static_cast< float >(event.xOffset_);
   io.MouseWheel += static_cast< float >(event.yOffset_);
}

void
EditorGUI::Init()
{
   InputManager::RegisterForInput(parent_.GetWindow().GetWindowHandle(), this);

   // Setup Dear ImGui context
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO& io = ImGui::GetIO();
   io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls NOLINT

   SetStyle();

   PrepareResources();
   PreparePipeline();
}

void
EditorGUI::Shutdown()
{
   ImGui::DestroyContext();
}

bool
EditorGUI::IsBlockingEvents()
{
   const ImGuiIO& io = ImGui::GetIO();
   return io.WantCaptureMouse || io.WantTextInput;
}

void
EditorGUI::UpdateUI()
{
   ImGuiIO& io = ImGui::GetIO();
   io.DisplaySize = ImVec2(parent_.GetWindowSize().x, parent_.GetWindowSize().y);

   ImGui::NewFrame();

   windowSize_ = parent_.GetWindowSize();

   windowWidth_ = windowSize_.x / 7;
   toolsWindowHeight_ = std::max(windowSize_.y / 20.0f, 64.0f);
   levelWindowHeight_ = windowSize_.y - toolsWindowHeight_;
   gameObjectWindowHeight_ = windowSize_.y;

   RenderMainPanel();

   if (currentLevel_)
   {
      RenderLevelMenu();
   }

   if (!parent_.GetSelectedObjects().empty())
   {
      RenderSelectedObjectsMenu();
   }
   else if (currentlySelectedGameObject_ != Object::INVALID_ID)
   {
      RenderGameObjectMenu();
   }

   if (exitPushed_)
   {
      RenderExitWindow();
   }

   ImGui::Render();

   setScrollTo_ = {};
}

void
EditorGUI::LevelLoaded(const std::shared_ptr< Level >& loadedLevel)
{
   currentLevel_ = loadedLevel;
   const auto& objects = currentLevel_->GetObjects();
   for (const auto& object : objects)
   {
      objectsInfo_[object.GetID()] = {
         fmt::format("[{}] {} ({:.2f}, {:.2f})", object.GetTypeString().c_str(),
                     object.GetName().c_str(), object.GetPosition().x, object.GetPosition().y),
         false};
   }

   const auto& enemies = currentLevel_->GetEnemies();
   for (const auto& enemy : enemies)
   {
      objectsInfo_[enemy.GetID()] = {
         fmt::format("[{}] {} ({:.2f}, {:.2f})", enemy.GetTypeString().c_str(),
                     enemy.GetName().c_str(), enemy.GetPosition().x, enemy.GetPosition().y),
         false};
   }

   const auto& player = currentLevel_->GetPlayer();
   if (player.GetID() != Object::INVALID_ID)
   {
      objectsInfo_[player.GetID()] = {
         fmt::format("[{}] {} ({:.2f}, {:.2f})", player.GetTypeString().c_str(),
                     player.GetName().c_str(), player.GetPosition().x, player.GetPosition().y),
         false};
   }
}

void
EditorGUI::ObjectSelected(Object::ID ID, bool groupSelect)
{
   objectsInfo_[ID].second = true;
   setScrollTo_ = ID;

   const auto& gameObject = parent_.GetLevel().GetGameObjectRef(ID);
   selectedObjects_.emplace_back(
      ID, gameObject.GetHasCollision(), gameObject.GetSprite().GetRenderInfo().layer);
   RecalculateCommonRenderLayerAndColision();

   if (not groupSelect)
   {
      currentlySelectedGameObject_ = ID;
   }
}

void
EditorGUI::ObjectUnselected(Object::ID ID)
{
   if (currentlySelectedGameObject_ == ID)
   {
      currentlySelectedGameObject_ = Object::INVALID_ID;
   }
   else
   {
      objectsInfo_[ID].second = false;

      selectedObjects_.erase(stl::find_if(selectedObjects_, [ID](const auto& obj) {
         const auto [id, collision, layer] = obj;
         return id == ID;
      }));
      RecalculateCommonRenderLayerAndColision();
   }
}

void
EditorGUI::ObjectUpdated(Object::ID ID)
{
   const auto& object = parent_.GetLevel().GetObjectRef(ID);

   switch (object.GetType())
   {
      case ObjectType::ENEMY:
      case ObjectType::PLAYER:
      case ObjectType::OBJECT: {
         const auto& gameObject = static_cast< const GameObject& >(object);

         objectsInfo_[ID].first = fmt::format(
            "[{}] {} ({:.2f}, {:.2f})", gameObject.GetTypeString().c_str(),
            gameObject.GetName().c_str(), gameObject.GetPosition().x, gameObject.GetPosition().y);
      }
      break;

      case ObjectType::EDITOR_OBJECT:
      case ObjectType::ANIMATION_POINT:
      case ObjectType::NONE:
      case ObjectType::PATHFINDER_NODE: {
         // We don't care
      }
   }
}

void
EditorGUI::ObjectDeleted(Object::ID /*ID*/)
{
}

} // namespace looper
