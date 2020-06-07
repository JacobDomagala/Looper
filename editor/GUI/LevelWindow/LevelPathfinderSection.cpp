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
LevelPathfinderSection::ObjectUpdated(dgame::Object::ID ID)
{
}

void
LevelPathfinderSection::ObjectDeleted(dgame::Object::ID ID)
{
   auto object = std::find_if(m_objects.begin(), m_objects.end(), [ID](auto& object) { return object->GetID() == ID; });

   if (object != m_objects.end())
   {
      switch ((*object)->GetType())
      {
         case dgame::Object::TYPE::PATHFINDER_NODE: {
            auto node = std::find_if(m_pathfinderNodes.begin(), m_pathfinderNodes.end(), [ID](auto& point) { return point.id == ID; });

            if (node != m_pathfinderNodes.end())
            {
               m_pathfinderNodesLayout->removeChild(node->m_xPos);
               m_pathfinderNodesLayout->removeChild(node->m_yPos);
               m_pathfinderNodesLayout->removeChild(node->m_removePoint);

               RemoveWidget(node->m_xPos);
               RemoveWidget(node->m_yPos);
               RemoveWidget(node->m_removePoint);

               m_objects.erase(object);
               m_pathfinderNodes.erase(node);
            }
         }
         break;

         case dgame::Object::TYPE::ENEMY: {
         }
         break;
      }
   }
}

} // namespace dgame