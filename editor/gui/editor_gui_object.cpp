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
      const auto& objectInfo = objectsInfo_.at(object->GetID());

      auto label = objectInfo.first;

      if (ImGui::Selectable(label.c_str(),
                            currentlySelectedGameObject_
                               ? currentlySelectedGameObject_->GetID() == object->GetID()
                               : false))
      {
         parent_.GetCamera().SetCameraAtPosition(object->GetPosition());
         parent_.HandleGameObjectSelected(object, true);

         // Don't make the UI jump
         setScrollTo_ = {};
      }
   }

   ImGui::EndChild();

   // If the user selects Objects from the List
   if (currentlySelectedGameObject_)
   {
      RenderGameObjectContent();
   }

   ImGui::End();
}

void
EditorGUI::RenderGameObjectContent()
{
   ImGui::SetNextItemOpen(true);
   if (ImGui::CollapsingHeader("General"))
   {
      DrawWidget("Name", [this]() {
         auto name = currentlySelectedGameObject_->GetName();
         const auto nameLength = 20;
         name.resize(nameLength);
         if (ImGui::InputText("##Name", name.data(), nameLength))
         {
            currentlySelectedGameObject_->SetName(name);
            objectsInfo_[currentlySelectedGameObject_->GetID()].first = fmt::format(
               "[{}] {} ({:.2f}, {:.2f})", currentlySelectedGameObject_->GetTypeString().c_str(),
               currentlySelectedGameObject_->GetName().c_str(),
               currentlySelectedGameObject_->GetPosition().x,
               currentlySelectedGameObject_->GetPosition().y);
         }
      });

      if (ImGui::BeginTable("ObjectTable", 2))
      {
         CreateActionRowLabel("RenderLayer", [this] {
            const auto items = std::to_array< std::string >(
               {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"});
            if (ImGui::BeginCombo(
                   "##combo",
                   fmt::format("{}",
                               currentlySelectedGameObject_->GetSprite().GetRenderInfo().layer)
                      .c_str()))
            {
               for (const auto& item : items)
               {
                  if (ImGui::Selectable(item.c_str()))
                  {
                     parent_.AddToWorkQueue([this, item] {
                        const auto layer = std::stoi(item);
                        const auto oldLayer =
                           currentlySelectedGameObject_->GetSprite().GetRenderInfo().layer;
                        currentlySelectedGameObject_->GetSprite().ChangeRenderLayer(layer);

                        renderer::VulkanRenderer::SetupVertexBuffer(oldLayer);
                        renderer::VulkanRenderer::SetupVertexBuffer(layer);
                     });
                  }
               }
               ImGui::EndCombo();
            }
         });
         CreateRow("Type", fmt::format("{}", currentlySelectedGameObject_->GetTypeString()));
         CreateRow("ID", fmt::format("{}", currentlySelectedGameObject_->GetID()));
         CreateActionRowLabel("Has Collision", [this] {
            auto collision = currentlySelectedGameObject_->GetHasCollision();
            if (ImGui::Checkbox("##Has Collision", &collision))
            {
               currentlySelectedGameObject_->SetHasCollision(collision);
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
      DrawWidget("Position", [this]() {
         auto objectPosition = currentlySelectedGameObject_->GetSprite().GetPosition();
         ImGui::InputFloat2("##Position", &objectPosition.x);
      });

      DrawWidget("Size", [this]() {
         auto sprite_size = currentlySelectedGameObject_->GetSprite().GetSize();
         if (ImGui::SliderFloat2("##Size", &sprite_size.x, 10, 1000))
         {
            currentlySelectedGameObject_->SetSize(sprite_size);
            parent_.GetLevel().UpdateCollisionTexture();
         }
      });

      DrawWidget("Rotate", [this]() {
         auto rotation = currentlySelectedGameObject_->GetSprite().GetRotation(
            renderer::Sprite::RotationType::DEGREES);
         if (ImGui::SliderFloat("##Rotate", &rotation,
                                glm::degrees(renderer::Sprite::ROTATION_RANGE.first),
                                glm::degrees(renderer::Sprite::ROTATION_RANGE.second)))
         {
            currentlySelectedGameObject_->Rotate(glm::radians(rotation));
         }
      });
   }

   BlankLine();

   ImGui::SetNextItemOpen(true);
   if (ImGui::CollapsingHeader("Shader"))
   {
      const auto sectionSize = ImGui::GetContentRegionAvail();
      const auto currentPos = ImGui::GetCursorScreenPos();
      ImGui::SetCursorScreenPos(ImVec2(currentPos.x + sectionSize.x / 4.0f, currentPos.y));
      ImGui::Image(static_cast< ImTextureID >(
                      GetDescriptor(currentlySelectedGameObject_->GetSprite().GetTexture()->GetID(),
                                    descriptorPool_, descriptorSetLayout_)),
                   {glm::min(sectionSize.x, 128.0f), glm::min(sectionSize.x, 128.0f)});

      if (ImGui::BeginTable("TextureInfoTable", 2))
      {
         auto& sprite = currentlySelectedGameObject_->GetSprite();
         CreateRow("Name", fmt::format("{}", sprite.GetTextureName()));
         CreateRow("ID", fmt::format("{}", sprite.GetTexture()->GetID()));
         CreateActionRowLabel("File", [this]() {
            auto& sprite = currentlySelectedGameObject_->GetSprite();

            ImGui::InputText("##Texture", sprite.GetTextureName().data(),
                             sprite.GetTextureName().size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::SameLine();

            if (ImGui::Button(ICON_FA_PENCIL ""))
            {
               auto textureName = FileManager::FileDialog(
                  IMAGES_DIR, {{"PNG texture", "png"}, {"JPEG texture", "jpg"}}, false);
               if (!textureName.empty())
               {
                  sprite.SetTextureFromFile(textureName);
               }
            }
         });
      }
      ImGui::EndTable();
   }

   if (currentlySelectedGameObject_->GetType() == ObjectType::ENEMY)
   {
      const auto animatablePtr =
         std::dynamic_pointer_cast< Animatable >(currentlySelectedGameObject_);

      BlankLine();

      ImGui::SetNextItemOpen(true);
      if (ImGui::CollapsingHeader("Animation"))
      {
         DrawWidget("Type", [animatablePtr]() {
            if (ImGui::RadioButton("Loop", animatablePtr->GetAnimationType()
                                              == Animatable::ANIMATION_TYPE::LOOP))
            {
               animatablePtr->SetAnimationType(Animatable::ANIMATION_TYPE::LOOP);
            }

            ImGui::SameLine();
            if (ImGui::RadioButton("Reversal", animatablePtr->GetAnimationType()
                                                  == Animatable::ANIMATION_TYPE::REVERSABLE))
            {
               animatablePtr->SetAnimationType(Animatable::ANIMATION_TYPE::REVERSABLE);
            }
         });

         bool animationVisible = animatablePtr->GetRenderAnimationSteps(); // NOLINT
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
            time::Timer::ConvertToMs(animatablePtr->GetAnimationDuration()).count();
         if (parent_.IsObjectAnimated())
         {
            // timer += static_cast< float >(parent_.GetDeltaTime().count());
            // timer = glm::min(animationDuration, timer);
            timer = animatablePtr->GetTotalTimeElapsed().count();
         }

         ImGui::SameLine();
         if (ImGui::SliderFloat("##", &timer, 0.0f, animationDuration, "%.3f ms"))
         {
            currentlySelectedGameObject_->GetSprite().SetTranslateValue(glm::vec3(
               animatablePtr->SetAnimation(time::milliseconds(static_cast< uint64_t >(timer))),
               0.0f));
         }

         auto& animationPoints = animatablePtr->GetAnimationKeypoints();
         auto newNodePosition = currentlySelectedGameObject_->GetPosition();
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
