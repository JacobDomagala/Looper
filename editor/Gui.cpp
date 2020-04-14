#include "Gui.hpp"
#include "Editor.hpp"
#include "GameObject.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <nanovg.h>
#include <fstream>
#include <thread>

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
Gui::CreateLeftPanel()
{
   auto toolsWindow = CreateWindow(&m_parent, "TOOLS", glm::ivec2(0, 0), new nanogui::GroupLayout());
   m_windows.insert({TOOLS, toolsWindow});

   CreateLabel(toolsWindow, "Play current Level");
   auto playLevelButton = CreateButton(
      toolsWindow, "PlayLevel", [&] { m_parent.PlayLevel(); }, ENTYPO_ICON_ARROW_BOLD_RIGHT, false);


   CreateLabel(toolsWindow, "Level creation");
   auto openLoadLevelLayout =
      CreateLayout(toolsWindow, new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 6));

   CreateButton(
      openLoadLevelLayout, "Open",
      [&] {
         auto file = nanogui::file_dialog({{"dgl", "DGame Level"}}, false);
         if (!file.empty())
         {
            m_parent.LoadLevel(file);
            LevelLoaded();
         }
      },
      ENTYPO_ICON_NEWSLETTER);

   CreateButton(
      openLoadLevelLayout, "Save",
      [&] {
         std::filesystem::path file = nanogui::file_dialog({{"dgl", "DGame Level"}}, true);
         if (!file.empty())
         {
            const auto fileName = file.has_extension() ? file.u8string() : file.u8string() + ".dgl";
            m_parent.SaveLevel(fileName);
         }
      },
      ENTYPO_ICON_SAVE);

   CreateButton(
      toolsWindow, "Create",
      [&] {
         auto windowCenter = m_parent.GetWindowSize() / glm::vec2(2.0f);
         auto createLevelWindow = CreateWindow(&m_parent, "CREATE LEVEL", windowCenter, new nanogui::GridLayout());

         CreateLabel(createLevelWindow, "Width");
         auto widthTextBox = CreateTextBox(createLevelWindow, "3000");

         CreateLabel(createLevelWindow, "Height");
         auto heightTextBox = CreateTextBox(createLevelWindow, "3000");

         CreateButton(createLevelWindow, "Create", [&, createLevelWindow, widthTextBox, heightTextBox] {
            m_parent.CreateLevel(glm::ivec2(std::stoi(widthTextBox->value()), std::stoi(heightTextBox->value())));
            LevelLoaded();
            createLevelWindow->dispose();
         });
         CreateButton(createLevelWindow, "Cancel", [&, createLevelWindow] { createLevelWindow->dispose(); });
      },
      ENTYPO_ICON_DOCUMENT);

   CreateLabel(toolsWindow, "");
   CreateCheckBox(
      toolsWindow, [&](bool checked) { m_parent.ShowWireframe(checked); }, "Render wireframe");

   CreateLabel(toolsWindow, "");
   auto addObjectPopup = CreatePopupButton(toolsWindow, "Add", new nanogui::GroupLayout, ENTYPO_ICON_NEW, false);

   CreateButton(addObjectPopup.second, "Enemy", [&, addObjectPopup]() {
      m_parent.AddGameObject(GameObject::TYPE::ENEMY);
      addObjectPopup.first->setPushed(false);
   });

   CreateButton(addObjectPopup.second, "Player", [&, addObjectPopup]() {
      m_parent.AddGameObject(GameObject::TYPE::PLAYER);
      addObjectPopup.first->setPushed(false);
   });


   m_levelDependentWidgets.push_back(playLevelButton);
   m_levelDependentWidgets.push_back(addObjectPopup.first);
}

void
Gui::GameObjectSelected(std::shared_ptr< GameObject > selectedGameObject)
{
   auto textureButtonLambda = [&, selectedGameObject]() {
      auto filePath = nanogui::file_dialog({{"png", "Texture file"}}, false);
      if (!filePath.empty())
      {
         const auto fileName = std::filesystem::path(filePath).filename().u8string();
         selectedGameObject->GetSprite().SetTextureFromFile(fileName);
         m_currentGameObjectWindow.m_textureButton->setCaption(fileName);
      }
   };

   // Don't create new window for newly selected object
   // Just refill its content with new object's values
   if (m_windows.find(GAMEOBJECT) != m_windows.end())
   {
      const auto objectSize = selectedGameObject->GetSize();

      m_currentGameObjectWindow.m_objectWidth->setValue(std::to_string(objectSize.x));
      m_currentGameObjectWindow.m_objectHeight->setValue(std::to_string(objectSize.y));

      m_currentGameObjectWindow.m_textureButton->setCaption(selectedGameObject->GetSprite().GetTextureName());
      m_currentGameObjectWindow.m_textureButton->setCallback(textureButtonLambda);
   }
   else
   {
      const auto windowSize = m_parent.GetWindowSize();
      auto gameObjectWindow = CreateWindow(&m_parent, "Object", glm::ivec2(windowSize.x / 1.1f, 0), new nanogui::GroupLayout());

      m_windows.insert({GAMEOBJECT, gameObjectWindow});

      const auto objectSize = selectedGameObject->GetSize();

      CreateLabel(gameObjectWindow, "Width");
      m_currentGameObjectWindow.m_objectWidth = CreateTextBox(gameObjectWindow, std::to_string(objectSize.x));

      CreateLabel(gameObjectWindow, "Height");
      m_currentGameObjectWindow.m_objectHeight = CreateTextBox(gameObjectWindow, std::to_string(objectSize.y));

      const auto textureName = selectedGameObject->GetSprite().GetTextureName();
      CreateLabel(gameObjectWindow, "Texture");
      m_currentGameObjectWindow.m_textureButton = CreateButton(gameObjectWindow, textureName, textureButtonLambda);
   }
}

void
Gui::GameObjectUnselected()
{
   auto windowPtr = m_windows.at(GAMEOBJECT);
   windowPtr->dispose();
   m_windows.erase(GAMEOBJECT);
}

nanogui::Widget*
Gui::CreateLayout(nanogui::Widget* parent, nanogui::Layout* layout)
{
   auto tools = new nanogui::Widget(parent);
   tools->setLayout(layout);

   return tools;
}

nanogui::Window*
Gui::CreateWindow(nanogui::Widget* parent, const std::string& caption, const glm::ivec2& position, nanogui::Layout* layout)
{
   auto window = new nanogui::Window(parent, caption);
   window->setPosition(Eigen::Vector2i(position.x, position.y));
   window->setLayout(layout);

   return window;
}

nanogui::Label*
Gui::CreateLabel(nanogui::Widget* parent, const std::string& caption)
{
   return new nanogui::Label(parent, caption, "sans-bold");
}

nanogui::Button*
Gui::CreateButton(nanogui::Widget* parent, const std::string& caption, const std::function< void() >& callback, int icon, bool enabled)
{
   auto button = new nanogui::Button(parent, caption, icon);
   button->setCallback(callback);
   button->setEnabled(enabled);

   return button;
}

nanogui::TextBox*
Gui::CreateTextBox(nanogui::Widget* parent, const std::string& value, const glm::ivec2& size, bool editable)
{
   auto textBox = new nanogui::TextBox(parent);
   textBox->setEditable(editable);
   textBox->setFixedSize(nanogui::Vector2i(size.x, size.y));
   textBox->setValue(value);
   textBox->setDefaultValue("0.0");
   textBox->setFontSize(16);
   textBox->setFormat("[-]?[0-9]*\\.?[0-9]+");

   return textBox;
}

nanogui::CheckBox*
Gui::CreateCheckBox(nanogui::Widget* parent, const std::function< void(bool) >& callback, const std::string& text, float fontSize,
                    bool checked)
{
   auto checkBox = new nanogui::CheckBox(parent, text);
   checkBox->setFontSize(fontSize);
   checkBox->setChecked(checked);
   checkBox->setCallback(callback);

   return checkBox;
}

std::pair< nanogui::PopupButton*, nanogui::Popup* >
Gui::CreatePopupButton(nanogui::Widget* parent, const std::string& text, nanogui::Layout* layout, int icon, bool enabled)
{
   nanogui::PopupButton* popupBtn = new nanogui::PopupButton(parent, text, icon);
   popupBtn->setEnabled(enabled);

   nanogui::Popup* popup = popupBtn->popup();
   popup->setLayout(layout);

   return {popupBtn, popup};
}

void
Gui::LevelLoaded()
{
   for (auto& widget : m_levelDependentWidgets)
   {
      widget->setEnabled(true);
   }
}