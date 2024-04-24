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
#include <nlohmann/json.hpp>

#include <cstdint>
#include <set>

namespace looper {

EditorGUI::EditorGUI(Editor& parent) : parent_(parent)
{
}

void
EditorGUI::RecalculateCommonProperties()
{
   if (selectedObjects_.empty())
   {
      return;
   }

   const auto& [idFirst, collisionFirst, layerFirst, groupFirst] = selectedObjects_.front();
   commonRenderLayer_ = {true, layerFirst};
   commonCollision_ = {true, collisionFirst};
   commonGroup_ = {true, groupFirst};

   for (uint32_t idx = 1; idx < selectedObjects_.size(); idx++)
   {
      const auto& [id, collision, layer, group] = selectedObjects_.at(idx);
      if (commonRenderLayer_.first and (layer != commonRenderLayer_.second))
      {
         commonRenderLayer_.first = false;
      }

      if (commonCollision_.first and (collision != commonCollision_.second))
      {
         commonCollision_.first = false;
      }

      if (commonGroup_.first and (group != commonGroup_.second))
      {
         commonGroup_.first = false;
      }
   }
}

void
EditorGUI::UpdateGroupForSelection(const std::string& groupName)
{
   if (currentlySelectedGameObject_ != Object::INVALID_ID)
   {
      auto& objRef = parent_.GetLevel().GetGameObjectRef(currentlySelectedGameObject_);
      if (objRef.editorGroup_ != groupName)
      {
         auto& group = groups_.at(objRef.editorGroup_);
         group.erase(stl::find(group, currentlySelectedGameObject_));

         groups_.at(groupName).push_back(currentlySelectedGameObject_);
         objRef.editorGroup_ = groupName;
         stl::find_if(selectedObjects_, [this](const auto& obj) {
            return obj.ID == currentlySelectedGameObject_;
         })->group = groupName;
      }
   }
   else
   {
      for (auto& object : selectedObjects_)
      {
         auto& objRef = parent_.GetLevel().GetGameObjectRef(object.ID);
         if (objRef.editorGroup_ == groupName)
         {
            continue;
         }

         auto& group = groups_.at(object.group);
         group.erase(stl::find(group, object.ID));

         groups_.at(groupName).push_back(object.ID);
         object.group = groupName;
         objRef.editorGroup_ = groupName;
      }
   }

   RecalculateCommonProperties();
}

void
EditorGUI::DeleteGroup(const std::string& groupName)
{
   auto& objs = groups_.at(groupName);

   for (auto obj : objs)
   {
      auto& objRef = parent_.GetLevel().GetGameObjectRef(obj);
      objRef.editorGroup_ = "Default";
   }

   groups_.erase(groupName);
   groupNames_.erase(stl::find(groupNames_, groupName));
}

void
EditorGUI::CreateNewGroup()
{
   const auto halfSize = windowSize_ / 2.0f;

   static std::string name = "New Group";

   ImGui::SetNextWindowPos({halfSize.x - 160, halfSize.y - 40});
   ImGui::SetNextWindowSize({300, 140});
   ImGui::Begin("Create New Group", nullptr, ImGuiWindowFlags_NoResize);

   ImGui::Text("Name:");
   ImGui::Dummy(ImVec2(2.0f, 0.0f));
   ImGui::SameLine();
   ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.6f);

   ImGui::InputText("##NewGroupName", name.data(), name.capacity() + 1);

   ImGui::Dummy(ImVec2(0.0f, 5.0f));
   ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 10.0f, 0.0f));
   ImGui::SameLine();

   if (ImGui::Button("Create", {ImGui::GetWindowWidth() / 3.0f, 35}))
   {
      newGroupPushed_ = false;
      groupNames_.push_back(name);
      groups_[name] = {};
      UpdateGroupForSelection(name);
   }

   ImGui::SameLine();
   ImGui::Dummy(ImVec2(2.0f, 0.0f));
   ImGui::SameLine();
   if (ImGui::Button("Cancel", {ImGui::GetWindowWidth() / 3.0f, 35}))
   {
      newGroupPushed_ = false;
   }

   ImGui::End();
}

void
EditorGUI::EditGroup(const std::string& oldName)
{
   const auto halfSize = windowSize_ / 2.0f;

   ImGui::SetNextWindowPos({halfSize.x - 160, halfSize.y - 40});
   ImGui::SetNextWindowSize({300, 140});
   ImGui::Begin("Edit Group", nullptr, ImGuiWindowFlags_NoResize);

   ImGui::Text("Name:");
   ImGui::Dummy(ImVec2(2.0f, 0.0f));
   ImGui::SameLine();
   ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.6f);

   static std::string newName = oldName;
   ImGui::InputText("##NewGroupName", newName.data(), newName.capacity() + 1);

   ImGui::Dummy(ImVec2(0.0f, 5.0f));
   ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 10.0f, 0.0f));
   ImGui::SameLine();

   if (ImGui::Button("Rename", {ImGui::GetWindowWidth() / 3.0f, 35}))
   {
      renameGroupPushed_ = false;
      const auto& objects = groups_.at(oldName);
      for (auto obj : objects)
      {
         parent_.GetLevel().GetGameObjectRef(obj).editorGroup_ = newName;
      }

      *(stl::find(groupNames_, oldName)) = newName;

      groups_[newName] = groups_.at(oldName);
      groups_.erase(oldName);
   }

   ImGui::SameLine();
   ImGui::Dummy(ImVec2(2.0f, 0.0f));
   ImGui::SameLine();
   if (ImGui::Button("Cancel", {ImGui::GetWindowWidth() / 3.0f, 35}))
   {
      renameGroupPushed_ = false;
   }

   ImGui::End();
}

void
EditorGUI::KeyCallback(KeyEvent& event)
{
   auto& io = ImGui::GetIO();

   if (io.WantCaptureKeyboard)
   {
      const auto imguiKey = KeyToImGuiKey(event.key_);
      io.AddKeyEvent(imguiKey, (event.action_ == GLFW_PRESS));
      event.handled_ = true;
   }
}

void
EditorGUI::CharCallback(CharEvent& event)
{
   auto& io = ImGui::GetIO();
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

      groups_[object.editorGroup_].emplace_back(object.GetID());
   }

   const auto& enemies = currentLevel_->GetEnemies();
   for (const auto& enemy : enemies)
   {
      objectsInfo_[enemy.GetID()] = {
         fmt::format("[{}] {} ({:.2f}, {:.2f})", enemy.GetTypeString().c_str(),
                     enemy.GetName().c_str(), enemy.GetPosition().x, enemy.GetPosition().y),
         false};

      groups_[enemy.editorGroup_].emplace_back(enemy.GetID());
   }

   const auto& player = currentLevel_->GetPlayer();
   if (player.GetID() != Object::INVALID_ID)
   {
      objectsInfo_[player.GetID()] = {
         fmt::format("[{}] {} ({:.2f}, {:.2f})", player.GetTypeString().c_str(),
                     player.GetName().c_str(), player.GetPosition().x, player.GetPosition().y),
         false};

      groups_[player.editorGroup_].emplace_back(player.GetID());
   }

   LoadConfigFile();
}

void
EditorGUI::LoadConfigFile()
{
   auto fileName = parent_.GetLevelFileName();

   // Load the editor configuration file
   auto json = FileManager::LoadJsonFile(
      fmt::format("{}.editor.dgl", fileName.substr(0, fileName.size() - 4)));

   groupNames_ = json["GROUPS"];
}

void
EditorGUI::SaveConfigFile()
{
   nlohmann::json json;

   json["GROUPS"] = groupNames_;

   auto fileName = parent_.GetLevelFileName();
   FileManager::SaveJsonFile(fmt::format("{}.editor.dgl", fileName.substr(0, fileName.size() - 4)),
                             json);
}

void
EditorGUI::ObjectSelected(Object::ID ID, bool groupSelect)
{
   objectsInfo_[ID].selected = true;
   setScrollTo_ = ID;

   const auto& gameObject = parent_.GetLevel().GetGameObjectRef(ID);
   selectedObjects_.emplace_back(SelectedObjectInfo{ID, gameObject.GetHasCollision(),
                                                    gameObject.GetSprite().GetRenderInfo().layer,
                                                    gameObject.editorGroup_});


   RecalculateCommonProperties();

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
      objectsInfo_[ID].selected = false;

      selectedObjects_.erase(stl::find_if(selectedObjects_, [ID](const auto& obj) {
         const auto& [id, collision, layer, group] = obj;
         return id == ID;
      }));
      RecalculateCommonProperties();
   }

   if (selectedObjects_.empty())
   {
      selectedGroup_ = "";
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

         objectsInfo_[ID].description = fmt::format(
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

void
EditorGUI::ObjectAdded(Object::ID ID)
{
   const auto& object = static_cast< GameObject& >(parent_.GetLevel().GetObjectRef(ID));
   objectsInfo_[ID] = {fmt::format("[{}] {} ({:.2f}, {:.2f})", object.GetTypeString().c_str(),
                                   object.GetName().c_str(), object.GetPosition().x,
                                   object.GetPosition().y),
                       true};

   groups_["Default"].push_back(ID);
}

} // namespace looper
