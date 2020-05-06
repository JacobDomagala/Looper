#include "LevelPathfinderSection.hpp"
#include "Animatable.hpp"
#include "Editor.hpp"
#include "GameObject.hpp"
#include "GuiBuilder.hpp"
#include "Utils.hpp"

namespace dgame {

LevelPathfinderSection::LevelPathfinderSection(Widget* parent, Editor& editor, bool activeByDefault) : Section(parent, editor, "PATHFINDER")
{
}

void
LevelPathfinderSection::Create(std::shared_ptr< Level > level)
{
   m_loadedLevel = level;

   auto layout = GuiBuilder::CreateLayout(parent(), GuiBuilder::LayoutType::GRID, Orientation::Horizontal, 2, Alignment::Middle, 2, 2);

   m_showWayPoints = GuiBuilder::CreateCheckBox(
      layout, [&](bool checked) { m_editor.ShowWaypoints(checked); }, "Draw nodes", 16, true);
   AddWidget(m_showWayPoints);

   const auto fixtedWidth = parent()->fixedSize().x() / 5;
   const auto pathfinderNodes = m_loadedLevel->GetPathfinder().GetAllNodes();
   const auto levelSize = m_loadedLevel->GetSize();

   m_showNodesButton = GuiBuilder::CreatePopupButton(layout, "Nodes");
   m_showNodesButton.first->setFixedWidth(parent()->fixedSize().x() / 3);
   AddWidget(m_showNodesButton.first);

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
LevelPathfinderSection::Update()
{
}

void
LevelPathfinderSection::LevelLoaded(std::shared_ptr< Level > level)
{
   m_loadedLevel = level;
   Update();
}

void
LevelPathfinderSection::ObjectUpdated(int ID)
{
}

} // namespace dgame