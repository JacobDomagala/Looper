#include "EditorGUI.hpp"
#include "Animatable.hpp"
#include "Editor.hpp"
#include "GameObject.hpp"
#include "Utils.hpp"

#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include <imgui.h>

namespace dgame {

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
   ImGui_ImplOpenGL3_Init("#version 410");
}

bool
EditorGUI::OnEvent(const Event& e)
{
   const auto mousePos = InputManager::GetMousePos();
   const auto windowSize = m_parent.GetWindowSize();
   return (mousePos.x < m_windowWidth) || (mousePos.x > (windowSize.x - m_windowWidth));
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
   ImGui::PushStyleColor(ImGuiCol_Button, {0.45f, 0.0f, 0.2f, 0.8f});
   if (ImGui::Button("Play"))
   {
      m_parent.PlayLevel();
   }
   ImGui::PopStyleColor(1);
   ImGui::SameLine();
   if (ImGui::Button("Save"))
   {
      auto levelName = file_dialog({{"dgl", "DGame Level"}}, true);
      if (!levelName.empty())
      {
         m_parent.SaveLevel(levelName);
      }
   }
   ImGui::SameLine();
   if (ImGui::Button("Load"))
   {
      auto levelName = file_dialog({{"dgl", "DGame Level"}}, false);
      if (!levelName.empty())
      {
         m_parent.LoadLevel(levelName);
      }
   }
   ImGui::SameLine();
   if (ImGui::Button("Create"))
   {
      m_parent.CreateLevel({3000, 3000});
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
         auto size = m_currentLevel->GetSize();
         if (ImGui::InputInt2("Size", &size.x))
         {
            m_currentLevel->GetSprite().SetSize(size);
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
         static bool renderPathfinderNodes = false;
         static int gridDensity = 20;
         ImGui::Checkbox("Render nodes", &renderPathfinderNodes);
         ImGui::SameLine();
         if (ImGui::Button("Generate Nodes"))
         {
            m_parent.GeneratePathfinder(gridDensity);
         }
         ImGui::SameLine();
         ImGui::InputInt("density", &gridDensity);
      }

      ImGui::SetNextTreeNodeOpen(true);
      if (ImGui::CollapsingHeader("Shader"))
      {
      }

      ImGui::SetNextTreeNodeOpen(true);
      if (ImGui::CollapsingHeader("Objects"))
      {
         static int selected = 0;
         auto gameObjects = m_currentLevel->GetObjects();
         
         ImGui::BeginChild("Loaded Objects", {0, 100}, true);
         for (auto& object : gameObjects)
         {
            char label[128];
            sprintf(label, "[%s] Name=%s Pos(%d,%d)", object->GetTypeString().c_str(), object->GetName().c_str(),
                    object->GetLocalPosition().x, object->GetLocalPosition().y);
            if (ImGui::Selectable(label))
            {
               m_parent.GetCamera().SetCameraAtPosition(object->GetLocalPosition());
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
      ImGui::Text("Camera Position %f %f", cameraPos.x, cameraPos.y);
      const auto cursorOpengGLPos = m_parent.ScreenToGlobal(InputManager::GetMousePos());
      ImGui::Text("Cursor Position %f %f", cursorOpengGLPos.x, cursorOpengGLPos.y);
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

         ImGui::InputText("Type", &type[0], 10, ImGuiInputTextFlags_ReadOnly);

         if (ImGui::InputText("Name", &name[0], nameLength))
         {
            m_currentlySelectedGameObject->SetName(name);
         }
      }

      ImGui::SetNextTreeNodeOpen(true);
      if (ImGui::CollapsingHeader("Transform"))
      {
         auto objectPosition = m_currentlySelectedGameObject->GetLocalPosition();
         auto size = m_currentlySelectedGameObject->GetSprite().GetSize();
         auto rotation = m_currentlySelectedGameObject->GetSprite().GetRotation(Sprite::RotationType::DEGREES);

         ImGui::InputInt2("Position", &objectPosition.x);

         if (ImGui::SliderInt2("Size", &size.x, 10, 500))
         {
            m_currentlySelectedGameObject->SetSize(size);
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
            ImGui::Image((ImTextureID)m_currentlySelectedGameObject->GetSprite().GetTexture().GetTextureHandle(), {150, 150});
         }
      }

      if (m_currentlySelectedGameObject->GetType() != GameObject::TYPE::PLAYER)
      {
         const auto animatablePtr = std::dynamic_pointer_cast< Animatable >(m_currentlySelectedGameObject);

         ImGui::SetNextTreeNodeOpen(true);
         if (ImGui::CollapsingHeader("Animation"))
         {
            ImGui::Text("Type");
            ImGui::SameLine();

            if (ImGui::RadioButton("Loop", animatablePtr->GetAnimationType() == Animatable::ANIMATION_TYPE::LOOP))
            {
               animatablePtr->SetAnimationType(Animatable::ANIMATION_TYPE::LOOP);
            }

            ImGui::SameLine();
            if (ImGui::RadioButton("Reversal", animatablePtr->GetAnimationType() == Animatable::ANIMATION_TYPE::REVERSABLE))
            {
               animatablePtr->SetAnimationType(Animatable::ANIMATION_TYPE::REVERSABLE);
            }

            static bool animationVisible = animatablePtr->GetRenderAnimationSteps();
            if (ImGui::Checkbox("Animation points visible", &animationVisible))
            {
               m_parent.SetRenderAnimationPoints(animationVisible);
            }

            if (ImGui::Button("Animate"))
            {
               m_parent.ToggleAnimateObject();
            }


            static float timer = 0.0f;
            const auto animationDuration = static_cast< float >(Timer::ConvertToMs(animatablePtr->GetAnimationDuration()).count());
            if (m_parent.IsObjectAnimated())
            {
               timer += m_parent.GetDeltaTime().count();
               timer = glm::min(animationDuration, timer);
            }

            ImGui::SameLine();
            if (ImGui::SliderFloat("", &timer, 0.0f, animationDuration, "%.3f ms"))
            {
               m_currentlySelectedGameObject->GetSprite().SetTranslateValue(
                  animatablePtr->SetAnimation(Timer::milliseconds(static_cast< uint64_t >(timer))));
            }

            static int selected = 0;
            const auto animationPoints = animatablePtr->GetAnimationKeypoints();
            ImGui::BeginChild("Animation Points", {0, 100}, true);
            for (int i = 0; i < animationPoints.size(); ++i)
            {
               char label[128];
               const auto& node = animationPoints[i];
               sprintf(label, "[%d] Pos(%.1f,%.1f) Time=%ds", i, node->m_end.x, node->m_end.y, node->m_timeDuration.count());
               if (ImGui::Selectable(label))
               {
                  m_parent.GetCamera().SetCameraAtPosition(node->m_end);
                  m_parent.HandleObjectSelected(node->GetID(), true);
               }
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

void
EditorGUI::EditorObjectSelected(std::shared_ptr< EditorObject > object)
{
   // m_currentlySelectedEditorObject = object;
}

void
EditorGUI::EditorObjectUnselected()
{
   // m_currentlySelectedEditorObject = nullptr;
}

void
EditorGUI::LevelLoaded(std::shared_ptr< Level > levelLoaded)
{
   m_currentLevel = levelLoaded;
}

void
EditorGUI::ObjectUpdated(Object::ID ID)
{
}

void
EditorGUI::ObjectDeleted(Object::ID ID)
{
}

} // namespace dgame