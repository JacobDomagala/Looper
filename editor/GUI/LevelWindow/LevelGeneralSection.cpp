#include "LevelGeneralSection.hpp"
#include "Animatable.hpp"
#include "Editor.hpp"
#include "GameObject.hpp"
#include "GuiBuilder.hpp"
#include "Utils.hpp"

namespace dgame {

LevelGeneralSection::LevelGeneralSection(Widget* parent, Editor& editor, bool activeByDefault) : Section(parent, editor, "GENERAL")
{
}

void
LevelGeneralSection::Create(std::shared_ptr< Level > level)
{
   m_loadedLevel = level;
   const auto objectSize = glm::ivec2(m_loadedLevel->GetSize());

   auto layout = GuiBuilder::CreateLayout(parent(), GuiBuilder::LayoutType::GRID);

   AddWidget(GuiBuilder::CreateLabel(layout, "Width"));

   m_width = GuiBuilder::CreateTextBox(layout, std::to_string(objectSize.x));
   AddWidget(m_width);

   AddWidget(GuiBuilder::CreateLabel(layout, "Height"));

   m_height = GuiBuilder::CreateTextBox(layout, std::to_string(objectSize.y));
   AddWidget(m_height);

   GuiBuilder::CreateLabel(parent(), "");
   AddWidget(GuiBuilder::CreateCheckBox(
      parent(), [&](bool checked) { m_editor.ShowWireframe(checked); }, "Render wireframe"));

   const auto fixtedWidth = parent()->fixedSize().x() / 5;
   const auto pathfinderNodes = m_loadedLevel->GetPathfinder().GetAllNodes();
   const auto levelSize = m_loadedLevel->GetSize();

   m_showNodesButton = GuiBuilder::CreatePopupButton(parent(), "Pathfinder nodes");
   m_pathfinderNodesLayout =
      GuiBuilder::CreateLayout(m_showNodesButton.second, GuiBuilder::LayoutType::GRID, Orientation::Horizontal, 3, Alignment::Middle, 2, 2);

   AddWidget(GuiBuilder::CreateLabel(m_pathfinderNodesLayout, "X"));
   AddWidget(GuiBuilder::CreateLabel(m_pathfinderNodesLayout, "Y"));
   AddWidget(GuiBuilder::CreateLabel(m_pathfinderNodesLayout, ""));

   for (auto& node : pathfinderNodes)
   {
      auto xValue = GuiBuilder::CreateFloatingPointBox(m_pathfinderNodesLayout, node->m_position.x, std::make_pair(0, levelSize.x),
                                                       [node](const std::string& val) {
                                                          node->m_position.x = std::stof(val);
                                                          return true;
                                                       },
                                                       {fixtedWidth, 0});

      auto yValue = GuiBuilder::CreateFloatingPointBox(m_pathfinderNodesLayout, node->m_position.y, std::make_pair(0, levelSize.y),
                                                       [node](const std::string& val) {
                                                          node->m_position.y = std::stof(val);
                                                          return true;
                                                       },
                                                       {fixtedWidth, 0});

      auto removePoint = GuiBuilder::CreateButton(
         m_pathfinderNodesLayout, "", []() {}, ENTYPO_ICON_TRASH, fixtedWidth);

      m_pathfinderNodes.push_back({node->GetID(), xValue, yValue, removePoint});

      AddWidget(xValue);
      AddWidget(yValue);
      AddWidget(removePoint);

      if (m_objects.end() == std::find(m_objects.begin(), m_objects.end(), node))
      {
         m_objects.push_back(node);
      }
   }
}

void
LevelGeneralSection::Update()
{
   const auto objectSize = m_loadedLevel->GetSize();

   m_width->setValue(std::to_string(objectSize.x));
   m_height->setValue(std::to_string(objectSize.y));
}

void
LevelGeneralSection::LevelLoaded(std::shared_ptr< Level > level)
{
   m_loadedLevel = level;
   Update();
}

void
LevelGeneralSection::ObjectUpdated(int ID)
{
}

} // namespace dgame