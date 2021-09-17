#include "EditorGUI.hpp"
#include "Animatable.hpp"
#include "Editor.hpp"
#include "GameObject.hpp"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <fmt/format.h>
#include <imgui.h>
#include <imgui_internal.h>

namespace dgame {

static inline void
SetStyle()
{
   ImGuiStyle& style = ImGui::GetStyle();
   auto* colors = style.Colors;

   /// 0 = FLAT APPEARENCE
   /// 1 = MORE "3D" LOOK
   int is3D = 1;

   colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
   colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
   colors[ImGuiCol_ChildBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
   colors[ImGuiCol_WindowBg] = ImVec4(0.025f, 0.025f, 0.025f, 1.00f);
   colors[ImGuiCol_PopupBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
   colors[ImGuiCol_Border] = ImVec4(0.12f, 0.12f, 0.12f, 0.71f);
   colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
   colors[ImGuiCol_FrameBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.54f);
   colors[ImGuiCol_FrameBgHovered] = ImVec4(0.42f, 0.42f, 0.42f, 0.40f);
   colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.67f);
   colors[ImGuiCol_TitleBg] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
   colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
   colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05f, 0.05f, 0.05f, 0.90f);
   colors[ImGuiCol_MenuBarBg] = ImVec4(0.335f, 0.335f, 0.335f, 1.000f);
   colors[ImGuiCol_ScrollbarBg] = ImVec4(0.24f, 0.24f, 0.24f, 0.53f);
   colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
   colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
   colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
   colors[ImGuiCol_CheckMark] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
   colors[ImGuiCol_SliderGrab] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
   colors[ImGuiCol_SliderGrabActive] = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
   colors[ImGuiCol_Button] = ImVec4(0.54f, 0.54f, 0.54f, 0.35f);
   colors[ImGuiCol_ButtonHovered] = ImVec4(0.52f, 0.52f, 0.52f, 0.59f);
   colors[ImGuiCol_ButtonActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
   colors[ImGuiCol_Header] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
   colors[ImGuiCol_HeaderHovered] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
   colors[ImGuiCol_HeaderActive] = ImVec4(0.76f, 0.76f, 0.76f, 0.77f);
   colors[ImGuiCol_Separator] = ImVec4(0.000f, 0.000f, 0.000f, 0.137f);
   colors[ImGuiCol_SeparatorHovered] = ImVec4(0.700f, 0.671f, 0.600f, 0.290f);
   colors[ImGuiCol_SeparatorActive] = ImVec4(0.702f, 0.671f, 0.600f, 0.674f);
   colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
   colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
   colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
   colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
   colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
   colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
   colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
   colors[ImGuiCol_TextSelectedBg] = ImVec4(0.73f, 0.73f, 0.73f, 0.35f);
   colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
   colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
   colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
   colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
   colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

   style.PopupRounding = 3;

   style.WindowPadding = ImVec2(4, 4);
   style.FramePadding = ImVec2(6, 4);
   style.ItemSpacing = ImVec2(6, 2);

   style.ScrollbarSize = 18;

   style.WindowBorderSize = 1;
   style.ChildBorderSize = 1;
   style.PopupBorderSize = 1;
   style.FrameBorderSize = static_cast< float >(is3D);

   style.WindowRounding = 3;
   style.ChildRounding = 3;
   style.FrameRounding = 3;
   style.ScrollbarRounding = 2;
   style.GrabRounding = 3;

#ifdef IMGUI_HAS_DOCK
   style.TabBorderSize = is3D;
   style.TabRounding = 3;

   colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
   colors[ImGuiCol_Tab] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
   colors[ImGuiCol_TabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
   colors[ImGuiCol_TabActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
   colors[ImGuiCol_TabUnfocused] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
   colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
   colors[ImGuiCol_DockingPreview] = ImVec4(0.85f, 0.85f, 0.85f, 0.28f);

   if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
   {
      style.WindowRounding = 0.0f;
      style.Colors[ImGuiCol_WindowBg].w = 1.0f;
   }
#endif
}

EditorGUI::EditorGUI(Editor& parent) : m_parent(parent)
{
}

void
EditorGUI::Init()
{
   // Setup Dear ImGui context
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO& io = ImGui::GetIO();
   io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

   // Setup Dear ImGui style
   ImGui::StyleColorsDark();

   ImGui_ImplGlfw_InitForOpenGL(m_parent.GetWindow().GetWindowHandle(), true);
   ImGui_ImplOpenGL3_Init("#version 450");
   SetStyle();
}

void
EditorGUI::Shutdown()
{
   ImGui_ImplGlfw_Shutdown();
   ImGui_ImplOpenGL3_Shutdown();
   ImGui::DestroyContext();
}

bool
EditorGUI::IsBlockingEvents()
{
   const ImGuiIO& io = ImGui::GetIO();
   return io.WantCaptureMouse || io.WantTextInput;
}

void
EditorGUI::Render()
{
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();

   const auto size = m_parent.GetWindowSize();

   m_windowWidth = size.x / 7;
   const auto toolsWindowHeight = 60;
   const auto levelWindowHeight = size.y - toolsWindowHeight;
   const auto gameObjectWindowHeight = size.y;
   const auto debugWindowWidth = size.x - 2 * m_windowWidth;
   const auto debugWindowHeight = 100;

   ImGui::SetNextWindowPos({0, 0});
   ImGui::SetNextWindowSize(ImVec2(m_windowWidth, toolsWindowHeight));
   ImGui::Begin("Tools");
   ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.45f, 0.0f, 0.2f, 0.8f});
   ImGui::BeginDisabled(m_currentLevel == nullptr);

   if (ImGui::Button("Play"))
   {
      m_parent.PlayLevel();
   }

   ImGui::PopStyleColor(1);
   ImGui::SameLine();
   if (ImGui::Button("Save"))
   {
      auto levelName = FileManager::FileDialog(LEVELS_DIR, {{"DGame Level file", "dgl"}}, true);
      if (!levelName.empty())
      {
         m_parent.SaveLevel(levelName);
      }
   }
   ImGui::EndDisabled();

   ImGui::SameLine();
   if (ImGui::Button("Load"))
   {
      auto levelName = FileManager::FileDialog(LEVELS_DIR, {{"DGame Level file", "dgl"}}, false);
      if (!levelName.empty())
      {
         m_parent.LoadLevel(levelName);
      }
   }
   ImGui::SameLine();
   if (ImGui::Button("Create"))
   {
      m_parent.CreateLevel({3072, 3072});
   }
   ImGui::End();

   if (m_currentLevel)
   {
      ImGui::SetNextWindowPos({0, toolsWindowHeight});
      ImGui::SetNextWindowSize(ImVec2(m_windowWidth, levelWindowHeight));
      ImGui::Begin("Level");

      ImGui::SetNextTreeNodeOpen(true);
      if (ImGui::CollapsingHeader("General"))
      {
         auto sprite_size = m_currentLevel->GetSprite().GetSize();
         if (ImGui::InputInt2("Size", &sprite_size.x))
         {
            m_currentLevel->SetSize(sprite_size);
         }

         auto [drawGrid, gridSize] = m_parent.GetGridData();
         if (ImGui::InputInt("cell size", &gridSize) || ImGui::Checkbox("Draw grid", &drawGrid))
         {
            m_parent.SetGridData(drawGrid, gridSize);
         }
      }

      ImGui::SetNextTreeNodeOpen(true);
      if (ImGui::CollapsingHeader("Pathfinder"))
      {
         static bool renderPathfinderNodes = m_parent.GetRenderNodes();
         if (ImGui::Checkbox("Render nodes", &renderPathfinderNodes))
         {
            m_parent.RenderNodes(renderPathfinderNodes);
         }
      }

      ImGui::SetNextTreeNodeOpen(true);
      if (ImGui::CollapsingHeader("Shader"))
      {
      }

      ImGui::SetNextTreeNodeOpen(true);
      if (ImGui::CollapsingHeader("Objects"))
      {
         const auto& gameObjects = m_currentLevel->GetObjects();

         const auto items = std::to_array< std::string >({"Enemy", "Player", "Object"});
         ImGui::SetNextItemWidth(m_windowWidth * 0.95f);

         // The second parameter is the label previewed before opening the combo.
         if (ImGui::BeginCombo("##combo", "Add"))
         {
            for (uint32_t n = 0; n < items.size(); n++)
            {
               if (ImGui::Selectable(items[n].c_str()))
               {
                  m_parent.AddGameObject(Object::GetTypeFromString(items[n]));
               }
            }
            ImGui::EndCombo();
         }

         ImGui::BeginChild("Loaded Objects", {0, 200}, true);

         for (auto& object : gameObjects)
         {
            auto label = fmt::format("[{}] {} ({}, {})", object->GetTypeString().c_str(),
                                     object->GetName().c_str(), object->GetPosition().x,
                                     object->GetPosition().y);

            if (ImGui::Selectable(label.c_str()))
            {
               m_parent.GetCamera().SetCameraAtPosition(object->GetPosition());
               m_parent.HandleGameObjectSelected(object, true);
            }
         }

         ImGui::EndChild();
      }


      ImGui::End();

      ImGui::SetNextWindowPos({m_windowWidth, size.y - debugWindowHeight});
      ImGui::SetNextWindowSize(ImVec2(debugWindowWidth, debugWindowHeight));
      ImGui::Begin("Debug");
      const auto cameraPos = m_parent.GetCamera().GetPosition();
      ImGui::Text("Camera Position %f, %f", static_cast< double >(cameraPos.x),
                  static_cast< double >(cameraPos.y));

      const auto cameraZoom = m_parent.GetCamera().GetZoomLevel();
      ImGui::Text("Camera Zoom %f", static_cast< double >(cameraZoom));

      const auto cursorOpengGLPos = m_parent.ScreenToGlobal(InputManager::GetMousePos());
      ImGui::Text("Cursor Position %f, %f", static_cast< double >(cursorOpengGLPos.x),
                  static_cast< double >(cursorOpengGLPos.y));

      auto& pathfinder = m_parent.GetLevel().GetPathfinder();
      const auto nodeID = pathfinder.GetNodeIDFromPosition(cursorOpengGLPos);
      Node curNode{};

      if (nodeID != -1)
      {
         curNode = pathfinder.GetNodeFromID(nodeID);
      }

      ImGui::Text("Cursor on tile ID = %d Coords(%d, %d)", curNode.m_ID, curNode.m_xPos,
                  curNode.m_yPos);

      ImGui::End();
   }

   if (m_currentlySelectedGameObject)
   {
      ImGui::SetNextWindowPos({size.x - m_windowWidth, 0});
      ImGui::SetNextWindowSize(ImVec2(m_windowWidth, gameObjectWindowHeight));
      ImGui::Begin("Game Object");
      ImGui::SetNextTreeNodeOpen(true);

      if (ImGui::CollapsingHeader("General"))
      {
         auto name = m_currentlySelectedGameObject->GetName();
         const auto nameLength = 20;
         name.resize(nameLength);

         auto type = m_currentlySelectedGameObject->GetTypeString();

         ImGui::InputText("Type", &type[0], type.size(), ImGuiInputTextFlags_ReadOnly);

         if (ImGui::InputText("Name", &name[0], nameLength))
         {
            m_currentlySelectedGameObject->SetName(name);
         }

         auto collision = m_currentlySelectedGameObject->GetHasCollision();
         if (ImGui::Checkbox("Has Collision", &collision))
         {
            m_currentlySelectedGameObject->SetHasCollision(collision);
         }
      }

      ImGui::SetNextTreeNodeOpen(true);
      if (ImGui::CollapsingHeader("Transform"))
      {
         auto objectPosition = m_currentlySelectedGameObject->GetPosition();
         auto sprite_size = m_currentlySelectedGameObject->GetSprite().GetSize();
         auto rotation =
            m_currentlySelectedGameObject->GetSprite().GetRotation(Sprite::RotationType::DEGREES);

         ImGui::InputFloat2("Position", &objectPosition.x);

         if (ImGui::SliderInt2("Size", &sprite_size.x, 10, 1000))
         {
            m_currentlySelectedGameObject->SetSize(sprite_size);
         }

         if (ImGui::SliderFloat("Rotate", &rotation, glm::degrees(Sprite::s_ROTATIONRANGE.first),
                                glm::degrees(Sprite::s_ROTATIONRANGE.second)))
         {
            m_currentlySelectedGameObject->Rotate(glm::radians(rotation));
         }
      }

      ImGui::SetNextTreeNodeOpen(true);
      if (ImGui::CollapsingHeader("Shader"))
      {
         if (m_currentLevel)
         {
            auto& sprite = m_currentlySelectedGameObject->GetSprite();
            ImGui::Image(reinterpret_cast< void* >(
                            static_cast< size_t >(sprite.GetTexture().GetTextureHandle())),
                         {150, 150});
            ImGui::InputText("FileName", &sprite.GetTextureName()[0],
                             sprite.GetTextureName().size(), ImGuiInputTextFlags_ReadOnly);
            if (ImGui::Button("Change Texture"))
            {
               auto textureName = FileManager::FileDialog(
                  IMAGES_DIR, {{"PNG texture", "png"}, {"JPEG texture", "jpg"}}, false);
               if (!textureName.empty())
               {
                  sprite.SetTextureFromFile(textureName);
               }
            }
         }
      }

      if (m_currentlySelectedGameObject->GetType() == GameObject::TYPE::ENEMY)
      {
         const auto animatablePtr =
            std::dynamic_pointer_cast< Animatable >(m_currentlySelectedGameObject);

         ImGui::SetNextTreeNodeOpen(true);
         if (ImGui::CollapsingHeader("Animation"))
         {
            ImGui::Text("Type");
            ImGui::SameLine();

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

            bool animationVisible = animatablePtr->GetRenderAnimationSteps();
            if (ImGui::Checkbox("Animation points visible", &animationVisible))
            {
               m_parent.SetRenderAnimationPoints(animationVisible);
            }

            if (ImGui::Button("Animate"))
            {
               m_parent.ToggleAnimateObject();
            }


            static float timer = 0.0f;
            const auto animationDuration = static_cast< float >(
               Timer::ConvertToMs(animatablePtr->GetAnimationDuration()).count());
            if (m_parent.IsObjectAnimated())
            {
               timer += static_cast< float >(m_parent.GetDeltaTime().count());
               timer = glm::min(animationDuration, timer);
            }

            ImGui::SameLine();
            if (ImGui::SliderFloat("", &timer, 0.0f, animationDuration, "%.3f ms"))
            {
               m_currentlySelectedGameObject->GetSprite().SetTranslateValue(
                  animatablePtr->SetAnimation(Timer::milliseconds(static_cast< uint64_t >(timer))));
            }

            // static int selected = 0;
            auto animationPoints = animatablePtr->GetAnimationKeypoints();
            auto newNodePosition = m_currentlySelectedGameObject->GetPosition();
            ImGui::BeginChild("Animation Points", {0, 100}, true);
            for (uint32_t i = 0; i < animationPoints.size(); ++i)
            {
               const auto& node = animationPoints[i];
               auto label = fmt::format("[{}] Pos({:.{}f},{:.{}f}) Time={}s", i, node.m_end.x, 1,
                                        node.m_end.y, 1, node.m_timeDuration.count());
               if (ImGui::Selectable(label.c_str()))
               {
                  m_parent.GetCamera().SetCameraAtPosition(node.m_end);
                  m_parent.HandleObjectSelected(node.GetID(), true);
                  m_parent.SetRenderAnimationPoints(true);
               }

               newNodePosition = node.m_end;
            }

            if (ImGui::Button("New"))
            {
               m_parent.GetCamera().SetCameraAtPosition(newNodePosition);
               m_parent.AddObject(Object::TYPE::ANIMATION_POINT);
               m_parent.SetRenderAnimationPoints(true);
            }
            ImGui::EndChild();
         }
      }

      ImGui::End();
   }


   ImGuiIO& io = ImGui::GetIO();
   io.DisplaySize = ImVec2(m_parent.GetWindowSize().x, m_parent.GetWindowSize().y);
   ImGui::Render();
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void
EditorGUI::GameObjectSelected(std::shared_ptr< GameObject > selectedGameObject)
{
   m_currentlySelectedGameObject = selectedGameObject;
}

void
EditorGUI::GameObjectUnselected()
{
   m_currentlySelectedGameObject = nullptr;
}

void EditorGUI::EditorObjectSelected(std::shared_ptr< EditorObject > /*object*/)
{
   // m_currentlySelectedEditorObject = object;
}

void
EditorGUI::EditorObjectUnselected()
{
   // m_currentlySelectedEditorObject = nullptr;
}

void
EditorGUI::LevelLoaded(std::shared_ptr< Level > loadedLevel)
{
   m_currentLevel = loadedLevel;
}

void EditorGUI::ObjectUpdated(Object::ID /*ID*/)
{
}

void EditorGUI::ObjectDeleted(Object::ID /*ID*/)
{
}

} // namespace dgame
