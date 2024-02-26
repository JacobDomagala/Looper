#include "animatable.hpp"
#include "editor.hpp"
#include "editor_gui.hpp"
#include "game_object.hpp"
#include "helpers.hpp"
#include "icons.hpp"
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

std::unordered_map< renderer::TextureID, VkDescriptorSet > textureDescriptors = {};

VkDescriptorSet
GetDescriptor(renderer::TextureID id, VkDescriptorPool descriptorPool,
              VkDescriptorSetLayout descriptorSetLayout)
{
   const auto desc = textureDescriptors.find(id);

   VkDescriptorSet descriptor{};
   if (desc != textureDescriptors.end())
   {
      descriptor = desc->second;
   }
   else
   {
      auto [view, sampler] = renderer::TextureLibrary::GetTexture(id)->GetImageViewAndSampler();
      descriptor = renderer::Texture::CreateDescriptorSet(sampler, view,
                                                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                          descriptorPool, descriptorSetLayout);

      textureDescriptors[id] = descriptor;
   }

   return descriptor;
}

void
EditorGUI::RenderSelectedObjectsMenu()
{
   ImGui::SetNextWindowPos({windowSize_.x - windowWidth_, 0});
   ImGui::SetNextWindowSize(ImVec2(windowWidth_, gameObjectWindowHeight_));
   ImGui::Begin("Selected Objects");
   ImGui::SetNextItemOpen(true);

   ImGui::BeginChild("Selected Objects", {0, 200}, true);

   const auto& gameObjects = parent_.GetSelectedObjects();

   for (const auto& object : gameObjects)
   {
      const auto& objectInfo = objectsInfo_.at(object);

      if (ImGui::Selectable(objectInfo.first.c_str(),
                            currentlySelectedGameObject_ != Object::INVALID_ID
                               ? currentlySelectedGameObject_ == object
                               : false))
      {
         const auto& gameObject = currentLevel_->GetGameObjectRef(object);
         parent_.GetCamera().SetCameraAtPosition(gameObject.GetPosition());
         parent_.HandleGameObjectClicked(object, false, true);

         // Don't make the UI jump
         setScrollTo_ = {};
      }
   }

   ImGui::EndChild();

   if (currentlySelectedGameObject_ != Object::INVALID_ID)
   {
      RenderGameObjectContent();
   }
   else
   {
      RenderGroupSelectModifications();
   }

   ImGui::End();
}

void
EditorGUI::RenderGroupSelectModifications()
{
   ImGui::SetNextItemOpen(true);
   if (ImGui::CollapsingHeader("Group Action"))
   {
      if (ImGui::BeginTable("ObjectsTable", 2))
      {
         // NOLINTNEXTLINE
         static int groupLayer = 0;
         CreateActionRowLabel("RenderLayer", [this] {
            const auto items = std::to_array< std::string >(
               {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"});
            if (ImGui::BeginCombo("##combo", fmt::format("{}", groupLayer).c_str()))
            {
               for (const auto& item : items)
               {
                  if (ImGui::Selectable(item.c_str()))
                  {
                     parent_.AddToWorkQueue([item, this] {
                        const auto layer = std::stoi(item);
                        const auto& gameObjects = parent_.GetSelectedObjects();
                        for (auto object : gameObjects)
                        {
                           parent_.GetLevel()
                              .GetGameObjectRef(object)
                              .GetSprite()
                              .ChangeRenderLayer(layer);
                        }
                     });
                  }
               }
               ImGui::EndCombo();
            }
         });
         // NOLINTNEXTLINE
         static bool groupHasCollision = false;
         CreateActionRowLabel("Has Collision", [this] {
            if (ImGui::Checkbox("##GroupHasCollision", &groupHasCollision))
            {
               parent_.AddToWorkQueue([this] {
                  const auto& gameObjects = parent_.GetSelectedObjects();
                  for (auto object : gameObjects)
                  {
                     parent_.GetLevel().GetGameObjectRef(object).SetHasCollision(groupHasCollision);
                  }
                  parent_.GetLevel().UpdateCollisionTexture();
               });
            }
         });

         ImGui::EndTable();
      }
   }
}

void
EditorGUI::RenderGameObjectContent()
{
   ImGui::SetNextItemOpen(true);
   if (ImGui::CollapsingHeader("Selected"))
   {
      auto& gameObject = currentLevel_->GetGameObjectRef(currentlySelectedGameObject_);

      DrawWidget("Name", [this, &gameObject]() {
         auto name = gameObject.GetName();
         const auto nameLength = 20;
         name.resize(nameLength);
         if (ImGui::InputText("##Name", name.data(), nameLength))
         {
            gameObject.SetName(name);
            objectsInfo_[currentlySelectedGameObject_].first =
               fmt::format("[{}] {} ({:.2f}, {:.2f})", gameObject.GetTypeString().c_str(),
                           gameObject.GetName().c_str(), gameObject.GetPosition().x,
                           gameObject.GetPosition().y);
         }
      });

      if (ImGui::BeginTable("ObjectTable", 2))
      {
         CreateActionRowLabel("RenderLayer", [this, &gameObject] {
            const auto items = std::to_array< std::string >(
               {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"});
            if (ImGui::BeginCombo(
                   "##combo",
                   fmt::format("{}", gameObject.GetSprite().GetRenderInfo().layer).c_str()))
            {
               for (const auto& item : items)
               {
                  if (ImGui::Selectable(item.c_str()))
                  {
                     parent_.AddToWorkQueue([&gameObject, item] {
                        const auto layer = std::stoi(item);
                        gameObject.GetSprite().ChangeRenderLayer(layer);
                     });
                  }
               }
               ImGui::EndCombo();
            }
         });
         CreateRow("Type", fmt::format("{}", gameObject.GetTypeString()));
         CreateRow("ID", fmt::format("{}", currentlySelectedGameObject_));
         CreateActionRowLabel("Has Collision", [this, &gameObject] {
            auto collision = gameObject.GetHasCollision();
            if (ImGui::Checkbox("##Has Collision", &collision))
            {
               gameObject.SetHasCollision(collision);
               parent_.GetLevel().UpdateCollisionTexture();
            }
         });

         ImGui::EndTable();
      }
   }

   BlankLine();

   ImGui::SetNextItemOpen(true);
   if (ImGui::CollapsingHeader("Transform"))
   {
      auto& gameObject = currentLevel_->GetGameObjectRef(currentlySelectedGameObject_);

      DrawWidget("Position", [&gameObject]() {
         auto objectPosition = gameObject.GetSprite().GetPosition();
         ImGui::InputFloat2("##Position", &objectPosition.x);
      });

      DrawWidget("Size", [this, &gameObject]() {
         auto sprite_size = gameObject.GetSprite().GetSize();
         if (ImGui::InputFloat2("##Size", &sprite_size.x))
         {
            gameObject.SetSize(sprite_size);
            parent_.GetLevel().UpdateCollisionTexture();
         }
      });

      DrawWidget("Rotate", [&gameObject]() {
         auto rotation = gameObject.GetSprite().GetRotation(renderer::RotationType::degrees);
         if (ImGui::InputFloat("##Rotate", &rotation,
                               glm::degrees(renderer::Sprite::ROTATION_RANGE.first),
                               glm::degrees(renderer::Sprite::ROTATION_RANGE.second)))
         {
            gameObject.Rotate(glm::radians(rotation));
         }
      });
   }

   BlankLine();

   ImGui::SetNextItemOpen(true);
   if (ImGui::CollapsingHeader("Texture"))
   {
      auto& gameObject = currentLevel_->GetGameObjectRef(currentlySelectedGameObject_);

      const auto sectionSize = ImGui::GetContentRegionAvail();
      const auto currentPos = ImGui::GetCursorScreenPos();
      ImGui::SetCursorScreenPos(ImVec2(currentPos.x + sectionSize.x / 4.0f, currentPos.y));
      ImGui::Image(
         static_cast< ImTextureID >(GetDescriptor(gameObject.GetSprite().GetTexture()->GetID(),
                                                  renderer::EditorData::descriptorPool_,
                                                  renderer::EditorData::descriptorSetLayout_)),
         {glm::min(sectionSize.x, 128.0f), glm::min(sectionSize.x, 128.0f)});

      if (ImGui::BeginTable("TextureInfoTable", 3))
      {
         const auto totalWidth = ImGui::GetContentRegionAvail().x;

         ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthStretch, 0.20f * totalWidth);
         ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.70f * totalWidth);
         ImGui::TableSetupColumn("Button", ImGuiTableColumnFlags_WidthStretch, 0.10f * totalWidth);


         auto& sprite = gameObject.GetSprite();
         CreateRow("Name", fmt::format("{}", sprite.GetTextureName()));
         BlankColumn();

         CreateRow("ID", fmt::format("{}", sprite.GetTexture()->GetID()));
         BlankColumn();

         CreateActionRowLabel(
            "File",
            [&gameObject]() {
               auto& sprite = gameObject.GetSprite();

               ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
               ImGui::InputText("##Texture", sprite.GetTextureName().data(),
                                sprite.GetTextureName().size(), ImGuiInputTextFlags_ReadOnly);
            },
            [&sprite] {
               if (ImGui::Button(ICON_FA_PENCIL "##ChangeTextureButton"))
               {
                  auto textureName = FileManager::FileDialog(
                     IMAGES_DIR, {{"PNG texture", "png"}, {"JPEG texture", "jpg"}}, false);
                  if (!textureName.empty())
                  {
                     sprite.SetTextureFromFile(textureName);
                  }
               }
            }

         );
      }
      ImGui::EndTable();
   }

   if (Object::GetTypeFromID(currentlySelectedGameObject_) == ObjectType::ENEMY)
   {
      auto& baseObject = currentLevel_->GetObjectRef(currentlySelectedGameObject_);
      auto& animatable = dynamic_cast< Animatable& >(baseObject);
      auto& gameObject = dynamic_cast< GameObject& >(baseObject);

      BlankLine();

      ImGui::SetNextItemOpen(true);
      if (ImGui::CollapsingHeader("Animation"))
      {
         DrawWidget("Type", [&animatable]() {
            if (ImGui::RadioButton("Loop", animatable.GetAnimationType()
                                              == Animatable::ANIMATION_TYPE::LOOP))
            {
               animatable.SetAnimationType(Animatable::ANIMATION_TYPE::LOOP);
            }

            ImGui::SameLine();
            if (ImGui::RadioButton("Reversal", animatable.GetAnimationType()
                                                  == Animatable::ANIMATION_TYPE::REVERSABLE))
            {
               animatable.SetAnimationType(Animatable::ANIMATION_TYPE::REVERSABLE);
            }
         });

         bool animationVisible = animatable.GetRenderAnimationSteps(); // NOLINT
         if (ImGui::Checkbox("Animation points visible", &animationVisible))
         {
            parent_.SetRenderAnimationPoints(animationVisible);
         }

         if (ImGui::Button("Animate"))
         {
            parent_.ToggleAnimateObject();
         }

         static float timer = 0.0f; // NOLINT
         const auto animationDuration =
            time::Timer::ConvertToMs(animatable.GetAnimationDuration()).count();
         if (parent_.IsObjectAnimated())
         {
            // timer += static_cast< float >(parent_.GetDeltaTime().count());
            // timer = glm::min(animationDuration, timer);
            timer = animatable.GetTotalTimeElapsed().count();
         }

         ImGui::SameLine();
         if (ImGui::SliderFloat("##", &timer, 0.0f, animationDuration, "%.3f ms"))
         {
            gameObject.GetSprite().SetTranslateValue(glm::vec3(
               animatable.SetAnimation(time::milliseconds(static_cast< uint64_t >(timer))), 0.0f));
         }

         auto& animationPoints = animatable.GetAnimationKeypoints();
         auto newNodePosition = gameObject.GetPosition();
         ImGui::BeginChild("Animation Points", {0, 100}, true);
         if (ImGui::BeginTable("AnimationPointTable", 2))
         {
            auto contentWidth = ImGui::GetContentRegionAvail().x;
            ImGui::TableSetupColumn("Column 1", ImGuiTableColumnFlags_WidthStretch,
                                    contentWidth * 0.95f);
            ImGui::TableSetupColumn("Column 2", ImGuiTableColumnFlags_WidthStretch,
                                    contentWidth * 0.05f);

            for (uint32_t i = 0; i < animationPoints.size(); ++i) // NOLINT
            {
               const auto& node = animationPoints[i];
               const auto label = fmt::format("[{}] Time={}s", i, node.m_timeDuration.count());

               ImGui::TableNextRow();
               ImGui::TableNextColumn();
               if (ImGui::Selectable(label.c_str()))
               {
                  parent_.SelectAnimationPoint(node);
               }
               ImGui::TableNextColumn();

               ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0f, 0.0f, 0.0f, 1.0f});
               if (ImGui::Selectable(fmt::format("{}##{}", ICON_FA_XMARK, i).c_str()))
               {
                  parent_.AddToWorkQueue([this, nodeID = node.GetID()] {
                     parent_.ActionOnObject(Editor::ACTION::REMOVE, nodeID);
                  });
               }
               ImGui::PopStyleColor(1);

               newNodePosition = node.m_end;
            }
         }
         ImGui::EndTable();

         if (ImGui::Button("New"))
         {
            parent_.AddToWorkQueue(
               [this, newNodePosition] { parent_.AddAnimationPoint(newNodePosition); });
         }
         ImGui::EndChild();

         const auto selectedID = parent_.GetSelectedEditorObject();
         if (Object::GetTypeFromID(selectedID) == ObjectType::ANIMATION_POINT)
         {
            BlankLine();
            ImGui::SetNextItemOpen(true);
            if (ImGui::CollapsingHeader("Selected point"))
            {
               auto& node =
                  dynamic_cast< AnimationPoint& >(parent_.GetLevel().GetObjectRef(selectedID));

               if (ImGui::BeginTable("AnimationPointTable", 2))
               {
                  const auto it =
                     stl::find_if(animationPoints, [selectedID](const auto& animationPoint) {
                        return animationPoint.GetID() == selectedID;
                     });
                  const auto idx = std::distance(animationPoints.begin(), it);

                  CreateRow("Idx", fmt::format("{}", idx));
                  CreateRow("Position", fmt::format("{:.2f},{:.2f}", node.m_end.x, node.m_end.y));

                  ImGui::EndTable();
               }

               DrawWidget(fmt::format("Duration (sec)", node.m_end.x, node.m_end.y), [&node]() {
                  auto seconds = static_cast< int32_t >(node.m_timeDuration.count());
                  if (ImGui::SliderInt("##distance", &seconds, 0, 10))
                  {
                     node.m_timeDuration = std::chrono::seconds(seconds);
                  }
               });
            }
         }
      }
   }
}

void
EditorGUI::RenderGameObjectMenu() // NOLINT
{
   ImGui::SetNextWindowPos({windowSize_.x - windowWidth_, 0});
   ImGui::SetNextWindowSize(ImVec2(windowWidth_, gameObjectWindowHeight_));
   ImGui::Begin("Game Object");

   RenderGameObjectContent();

   ImGui::End();
}

} // namespace looper
