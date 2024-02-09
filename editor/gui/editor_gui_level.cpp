#include "animatable.hpp"
#include "editor.hpp"
#include "editor_gui.hpp"
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

void
EditorGUI::RenderCreateNewLevelWindow()
{
   const auto halfSize = windowSize_ / 2.0f;
   std::unordered_map< std::string, glm::ivec2 > sizes = {{"Small", glm::ivec2{8192, 8192}},
                                                          {"Medium", glm::ivec2{16384, 16384}},
                                                          {"Large", glm::ivec2{65536, 65536}}};

   static std::string name = "DummyLevelName";
   static std::string currentSize = "Small";
   static glm::ivec2 size = sizes[currentSize];

   ImGui::SetNextWindowPos({halfSize.x - 160, halfSize.y - 60});
   ImGui::SetNextWindowSize({300, 180});
   ImGui::Begin("Create New", nullptr, ImGuiWindowFlags_NoResize);

   ImGui::Text("Size:");
   BlankLine(ImVec2(2.0f, 0.0f));
   ImGui::SameLine();
   ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.6f);

   const auto items = std::to_array< std::string >({"Small", "Medium", "Large"});
   // The second parameter is the label previewed before opening the combo.
   if (ImGui::BeginCombo("##combo", currentSize.c_str()))
   {
      for (const auto& item : items)
      {
         if (ImGui::Selectable(item.c_str()))
         {
            size = sizes.at(item);
            currentSize = item;
         }
      }
      ImGui::EndCombo();
   }

   ImGui::Dummy(ImVec2(0.0f, 5.0f));

   ImGui::Text("Name:");
   ImGui::Dummy(ImVec2(2.0f, 0.0f));
   ImGui::SameLine();
   ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.6f);

   ImGui::InputText("##Name", name.data(), name.capacity() + 1);

   ImGui::Dummy(ImVec2(0.0f, 5.0f));
   ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 10.0f, 0.0f));
   ImGui::SameLine();
   // ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 300) / 2);
   if (ImGui::Button("Create", {ImGui::GetWindowWidth() / 3.0f, 35}))
   {
      parent_.CreateLevel(name, size);
      createPushed_ = false;
   }
   ImGui::SameLine();
   ImGui::Dummy(ImVec2(2.0f, 0.0f));
   ImGui::SameLine();
   if (ImGui::Button("Cancel", {ImGui::GetWindowWidth() / 3.0f, 35}))
   {
      createPushed_ = false;
   }

   ImGui::End();
}

void
EditorGUI::RenderExitWindow()
{
   const auto halfSize = windowSize_ / 2.0f;

   ImGui::SetNextWindowPos({halfSize.x - 160, halfSize.y - 60});
   ImGui::SetNextWindowSize({240, 120});
   ImGui::Begin("Exit", nullptr, ImGuiWindowFlags_NoResize);

   ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.6f);

   ImGui::Text("Do you want to exit?");
   ImGui::Dummy(ImVec2(2.0f, 0.0f));

   ImGui::Dummy(ImVec2(0.0f, 5.0f));
   ImGui::Dummy(ImVec2(ImGui::GetWindowWidth() / 10.0f, 0.0f));
   ImGui::SameLine();
   // ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 300) / 2);
   if (ImGui::Button("Exit", {ImGui::GetWindowWidth() / 3.0f, 35}))
   {
      parent_.Shutdown();
   }
   ImGui::SameLine();
   ImGui::Dummy(ImVec2(2.0f, 0.0f));
   ImGui::SameLine();
   if (ImGui::Button("Cancel", {ImGui::GetWindowWidth() / 3.0f, 35}))
   {
      exitPushed_ = false;
   }

   ImGui::End();
}

void
EditorGUI::RenderMainPanel()
{
   ImGui::SetNextWindowPos({0, 0});
   ImGui::SetNextWindowSize(ImVec2(windowWidth_, toolsWindowHeight_));
   ImGui::Begin("Tools");

   if (ImGui::BeginTable("MainTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
   {
      CreateActionRow(
         [this] {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.0f, 0.5f, 0.0f, 0.8f});
            ImGui::BeginDisabled(currentLevel_ == nullptr);
            if (ImGui::Button(ICON_FA_PLAY "Play", ImVec2(-FLT_MIN, -FLT_MIN)))
            {
               parent_.AddToWorkQueue([this] { parent_.PlayLevel(); });
            }

            ImGui::PopStyleColor(1);
         },

         [this] {
            if (ImGui::Button("Save", ImVec2(-FLT_MIN, -FLT_MIN)))
            {
               auto levelName =
                  FileManager::FileDialog(LEVELS_DIR, {{"DGame Level file", "dgl"}}, true);
               if (!levelName.empty())
               {
                  parent_.AddToWorkQueue([this, levelName] { parent_.SaveLevel(levelName); });
               }
            }
            ImGui::EndDisabled();
         },

         [this] {
            if (ImGui::Button("Load", ImVec2(-FLT_MIN, -FLT_MIN)))
            {
               auto levelName =
                  FileManager::FileDialog(LEVELS_DIR, {{"DGame Level file", "dgl"}}, false);
               if (!levelName.empty())
               {
                  parent_.AddToWorkQueue([this, levelName] { parent_.LoadLevel(levelName); });
               }
            }
         },

         [this] {
            if (ImGui::Button("Create", ImVec2(-FLT_MIN, -FLT_MIN)) or createPushed_)
            {
               createPushed_ = true;
               RenderCreateNewLevelWindow();
            }
         });

      ImGui::EndTable();
   }


   ImGui::End();
}

void
EditorGUI::RenderLevelMenu() // NOLINT
{
   ImGui::SetNextWindowPos({0, toolsWindowHeight_});
   ImGui::SetNextWindowSize(ImVec2(windowWidth_, levelWindowHeight_));
   ImGui::Begin("Level");
   ImGui::SetNextItemOpen(true);
   if (ImGui::CollapsingHeader("General"))
   {
      if (ImGui::BeginTable("LevelTable", 2))
      {
         CreateRow("Size", fmt::format("{:.0f}, {:.0f}", currentLevel_->GetSprite().GetSize().x,
                                       currentLevel_->GetSprite().GetSize().y));

         CreateActionRowLabel("Render grid", [this] {
            auto [drawGrid, gridSize] = parent_.GetGridData();
            if (ImGui::Checkbox("##Render grid", &drawGrid))
            {
               parent_.SetGridData(drawGrid, gridSize);
            }
         });

         CreateActionRowLabel("Render collision", [this] {
            auto renderPathfinderNodes = parent_.GetRenderNodes();
            if (ImGui::Checkbox("##Render collision", &renderPathfinderNodes))
            {
               parent_.RenderNodes(renderPathfinderNodes);
            }
         });

         CreateActionRowLabel("RenderLayer", [this] {
            const auto items = std::to_array< std::string >(
               {"All", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"});
            const auto layer = parent_.GetRenderLayerToDraw();
            if (ImGui::BeginCombo(
                   "##combo",
                   fmt::format("{}", layer == -1 ? "All" : std::to_string(layer)).c_str()))
            {
               for (const auto& item : items)
               {
                  if (ImGui::Selectable(item.c_str()))
                  {
                     parent_.SetRenderLayerToDraw(item == "All" ? -1 : std::stoi(item));
                  }
               }
               ImGui::EndCombo();
            }
         });

         ImGui::EndTable();
      }
   }

   ImGui::SetNextItemOpen(true);
   if (ImGui::CollapsingHeader("Objects"))
   {
      const auto filterObjects = std::to_array< std::string >({"All", "Enemy", "Player", "Object"});

      // NOLINTBEGIN
      static std::string selectedFilter = filterObjects.at(0);
      static Object::ID searchID = 0;
      // NOLINTEND

      DrawWidget("Render by Type", [&filterObjects] {
         ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

         // The second parameter is the label previewed before opening the combo.
         if (ImGui::BeginCombo("##renderByType", selectedFilter.c_str()))
         {
            for (const auto& item : filterObjects)
            {
               if (ImGui::Selectable(item.c_str()))
               {
                  selectedFilter = item;
               }
            }
            ImGui::EndCombo();
         }
      });


      ImGui::BeginChild("Loaded Objects", {0, 200}, true);

      auto DisplayObject = [this](const auto& object) {
         const auto& objectInfo = objectsInfo_.at(object.GetID());

         if (ImGui::Selectable(objectInfo.first.c_str(), objectInfo.second))
         {
            parent_.GetCamera().SetCameraAtPosition(object.GetPosition());
            parent_.HandleGameObjectClicked(object.GetID(), false, true);

            // Don't make the UI jump
            setScrollTo_ = {};
         }

         if (setScrollTo_ == object.GetID())
         {
            // Scroll to make this widget visible
            ImGui::SetScrollHereY();
         }
      };

      if (selectedFilter == "Object")
      {
         for (const auto& object : currentLevel_->GetObjects())
         {
            DisplayObject(object);
         }
      }
      else if (selectedFilter == "Enemy")
      {
         for (const auto& object : parent_.GetLevel().GetEnemies())
         {
            DisplayObject(object);
         }
      }
      else if (selectedFilter == "Player")
      {
         DisplayObject(parent_.GetPlayer());
      }
      else if (selectedFilter == "All")
      {
         DisplayObject(parent_.GetPlayer());
         for (const auto& object : parent_.GetLevel().GetEnemies())
         {
            DisplayObject(object);
         }
         for (const auto& object : currentLevel_->GetObjects())
         {
            DisplayObject(object);
         }
      }

      ImGui::EndChild();

      if (ImGui::BeginTable("LevelTable", 3))
      {
         const auto totalWidth = ImGui::GetContentRegionAvail().x;

         ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, 0.35f * totalWidth);
         ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.55f * totalWidth);
         ImGui::TableSetupColumn("Button", ImGuiTableColumnFlags_WidthStretch, 0.10f * totalWidth);

         CreateActionRowLabel(
            "Search by ID",
            [] {
               ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
               ImGui::InputScalar("##searchByID", ImGuiDataType_U64, &searchID, nullptr, nullptr,
                                  nullptr, ImGuiInputTextFlags_EnterReturnsTrue);
            },
            [this] {
               if (ImGui::Button(ICON_FA_PLAY "##SearchByID"))
               {
                  if (currentlySelectedGameObject_ != searchID)
                  {
                     auto it = objectsInfo_.find(searchID);
                     if (it != objectsInfo_.end())
                     {
                        parent_.HandleObjectSelected(searchID, true);
                        parent_.GetCamera().SetCameraAtPosition(
                           parent_.GetLevel().GetGameObjectRef(searchID).GetPosition());
                     }
                  }
               }
            });

         // NOLINTNEXTLINE
         static std::string newObjectType = "Object";

         CreateActionRowLabel(
            "Add",
            [] {
               const auto items = std::to_array< std::string >({"Enemy", "Player", "Object"});

               // The second parameter is the label previewed before opening the combo.
               ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
               if (ImGui::BeginCombo("##combo2", newObjectType.c_str()))
               {
                  for (const auto& item : items)
                  {
                     if (ImGui::Selectable(item.c_str()))
                     {
                        newObjectType = item;
                     }
                  }
                  ImGui::EndCombo();
               }
            },
            [this] {
               if (ImGui::Button(ICON_FA_PLAY "##AddObject"))
               {
                  parent_.AddToWorkQueue([this] {
                     parent_.AddGameObject(Object::GetTypeFromString(newObjectType),
                                           parent_.GetCamera().GetPosition());
                     const auto objectID = parent_.GetSelectedGameObject();
                     const auto& object =
                        static_cast< GameObject& >(parent_.GetLevel().GetObjectRef(objectID));
                     objectsInfo_[objectID] = {
                        fmt::format("[{}] {} ({:.2f}, {:.2f})", object.GetTypeString().c_str(),
                                    object.GetName().c_str(), object.GetPosition().x,
                                    object.GetPosition().y),
                        true};
                  });
               }
            });
         ImGui::EndTable();
      }
   }

   BlankLine();

   ImGui::SetNextItemOpen(true);
   if (ImGui::CollapsingHeader("Debug"))
   {
      if (ImGui::BeginTable("DebugTable", 2))
      {
         CreateRow("FPS", fmt::format("{}", parent_.GetFramesLastSecond()));
         CreateRow("Frame time",
                   fmt::format("{:.2f}ms", parent_.GetFrameTime().GetMilliseconds().count()));
         CreateRow("UI update",
                   fmt::format("{:.2f}ms", parent_.GetUpdateUITime().GetMilliseconds().count()));
         CreateRow("Render",
                   fmt::format("{:.2f}ms", parent_.GetRenderTime().GetMilliseconds().count()));
         CreateRow("UI Render", fmt::format("{:.2f}ms", uiRenderTime.GetMilliseconds().count()));
         CreateRow("Number of objects", fmt::format("{}", parent_.GetLevel().GetNumOfObjects()));
         const auto cameraPos = parent_.GetCamera().GetPosition();
         CreateRow("Camera Position", fmt::format("{}", static_cast< glm::vec2 >(cameraPos)));
         CreateRow("Camera Zoom", fmt::format("{:.1f}", parent_.GetCamera().GetZoomLevel()));

         CreateRow("Camera Rotation", fmt::format("{:.1f}", parent_.GetCamera().GetRotation()));

         const auto cursorPos = parent_.ScreenToGlobal(InputManager::GetMousePos());
         CreateRow("Cursor Position", fmt::format("{}", cursorPos));

         if (parent_.GetLevel().IsInLevelBoundaries(cursorPos))
         {
            auto& pathfinder = parent_.GetLevel().GetPathfinder();
            const auto& curNode = pathfinder.GetNodeFromPosition(cursorPos);
            CreateRow("Cursor on TileID", fmt::format("{}", curNode.nodeId_));
            CreateRow("Cursor on Coords",
                      fmt::format("({}, {})", curNode.tile_.first, curNode.tile_.second));
         }
         else
         {
            CreateRow("Cursor on TileID", "INVALID");
            CreateRow("Cursor on Coords", "INVALID");
         }


         ImGui::EndTable();
      }
   }


   ImGui::End();
}

} // namespace looper
