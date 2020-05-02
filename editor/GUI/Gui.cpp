#include "Gui.hpp"
#include "Editor.hpp"
#include "GameObject.hpp"
#include "GameObjectWindow.hpp"
#include "GuiBuilder.hpp"
#include "LevelWindow.hpp"

#include <nanogui/vscrollpanel.h>

Gui::Gui(Editor& parent) : m_parent(parent)
{
}

void
Gui::Init()
{
   CreateLeftPanel();

   m_parent.setVisible(true);
}

void
Gui::Update()
{
   if (m_currentGameObjectWindow)
   {
      m_currentGameObjectWindow->Update();
   }

   if (m_levelWindow)
   {
      m_levelWindow->Update();
   }

   m_parent.performLayout();
}


void
Gui::CreateLeftPanel()
{
   auto toolsWindow = GuiBuilder::CreateWindow(&m_parent, "TOOLS", glm::ivec2(0, 0), new nanogui::GroupLayout());

   auto playLevelButton = CreatePlayButton(toolsWindow);

   auto openLoadLevelLayout = GuiBuilder::CreateLayout(toolsWindow, GuiBuilder::LayoutType::BOX);

   GuiBuilder::CreateButton(
      openLoadLevelLayout, "Open",
      [&] {
         auto file = nanogui::file_dialog({{"dgl", "DGame Level"}}, false);
         if (!file.empty())
         {
            m_parent.LoadLevel(file);
         }
      },
      ENTYPO_ICON_UPLOAD);

   auto saveLevelButton = GuiBuilder::CreateButton(
      openLoadLevelLayout, "Save",
      [&] {
         std::filesystem::path file = nanogui::file_dialog({{"dgl", "DGame Level"}}, true);
         if (!file.empty())
         {
            const auto fileName = file.has_extension() ? file.u8string() : file.u8string() + ".dgl";
            m_parent.SaveLevel(fileName);
         }
      },
      ENTYPO_ICON_DOWNLOAD, false);

   GuiBuilder::CreateButton(
      toolsWindow, "Create",
      [&] {
         auto windowCenter = m_parent.GetWindowSize() / glm::vec2(2.0f);
         auto createLevelWindow = GuiBuilder::CreateWindow(&m_parent, "CREATE LEVEL", windowCenter, new nanogui::GridLayout());

         GuiBuilder::CreateLabel(createLevelWindow, "Width");
         auto widthTextBox = GuiBuilder::CreateTextBox(createLevelWindow, "3000");

         GuiBuilder::CreateLabel(createLevelWindow, "Height");
         auto heightTextBox = GuiBuilder::CreateTextBox(createLevelWindow, "3000");

         GuiBuilder::CreateButton(createLevelWindow, "Create", [&, createLevelWindow, widthTextBox, heightTextBox] {
            m_parent.CreateLevel(glm::ivec2(std::stoi(widthTextBox->value()), std::stoi(heightTextBox->value())));
            createLevelWindow->dispose();
         });
         GuiBuilder::CreateButton(createLevelWindow, "Cancel", [&, createLevelWindow] { createLevelWindow->dispose(); });
      },
      ENTYPO_ICON_DOCUMENT);

   GuiBuilder::CreateLabel(toolsWindow, "");
   auto addObjectPopup = GuiBuilder::CreatePopupButton(toolsWindow, "Add", nanogui::Popup::Side::Right, new nanogui::GroupLayout,
                                                       ENTYPO_ICON_SQUARED_PLUS, false);
   addObjectPopup.first->setIconPosition(nanogui::Button::IconPosition::RightCentered);

   GuiBuilder::CreateButton(addObjectPopup.second, "Enemy", [&, addObjectPopup]() {
      m_parent.AddGameObject(GameObject::TYPE::ENEMY);
      addObjectPopup.first->setPushed(false);
   });

   GuiBuilder::CreateButton(addObjectPopup.second, "Player", [&, addObjectPopup]() {
      m_parent.AddGameObject(GameObject::TYPE::PLAYER);
      addObjectPopup.first->setPushed(false);
   });

   m_levelDependentWidgets.push_back(playLevelButton);
   m_levelDependentWidgets.push_back(saveLevelButton);
   m_levelDependentWidgets.push_back(addObjectPopup.first);
}

nanogui::Widget*
Gui::CreatePlayButton(nanogui::Widget* parent)
{
   auto playLevelButton = GuiBuilder::CreateButton(
      parent, "Play Level", [&] { m_parent.PlayLevel(); }, ENTYPO_ICON_ARROW_BOLD_RIGHT, 0, false);

   playLevelButton->setBackgroundColor(nanogui::Color(0.4f, 0.1f, 0.1f, 0.5));
   playLevelButton->setIconPosition(nanogui::Button::IconPosition::RightCentered);

   return playLevelButton;
}

void
Gui::GameObjectSelected(std::shared_ptr< GameObject > selectedGameObject)
{
   if (!m_currentGameObjectWindow)
   {
      // auto vscroll = new nanogui::VScrollPanel(addObjectPopup.second);
      m_currentGameObjectWindow = new GameObjectWindow(m_parent);
   }

   m_currentGameObjectWindow->GameObjectSelected(selectedGameObject);
}

void
Gui::GameObjectUnselected()
{
   m_currentGameObjectWindow->GameObjectUnselected();
}

void
Gui::EditorObjectSelected(int id)
{
   // m_currentGameObjectWindow->AnimationPointSelected(id);
}

void
Gui::EditorObjectUnselected(int id)
{
   // m_currentGameObjectWindow->AnimationPointUnselected(id);
}

void
Gui::LevelLoaded(std::shared_ptr< Level > levelLoaded)
{
   for (auto& widget : m_levelDependentWidgets)
   {
      widget->setEnabled(true);
   }

   // Level already loaded -> close level and object window
   if (m_levelWindow)
   {
      if (m_currentGameObjectWindow)
      {
         m_currentGameObjectWindow->GameObjectUnselected();
      }
   }
   else
   {
      m_levelWindow = new LevelWindow(m_parent);
   }

   m_levelWindow->LevelLoaded(levelLoaded);
}

void
Gui::ObjectUpdated(int ID)
{
   m_currentGameObjectWindow->ObjectUpdated(ID);
}