#include "Gui.hpp"
#include "Editor.hpp"

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
}

void
Gui::CreateLeftPanel()
{
   auto toolsWindow = CreateWindow(&m_parent, "TOOLS", glm::ivec2(0, 0), new nanogui::GroupLayout());

   CreateLabel(toolsWindow, "Play current Level");
   auto playLevelButton = CreateButton(
      toolsWindow, "PlayLevel", [&] { m_parent.PlayLevel(); }, false);

   CreateLabel(toolsWindow, "Open/Save Level");
   auto openLoadLevelLayout =
      CreateLayout(toolsWindow, new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 6));

   CreateButton(openLoadLevelLayout, "Open", [&, playLevelButton] {
      auto file = nanogui::file_dialog({{"dgl", "DGame Level"}}, false);
      m_parent.LoadLevel(file);
      playLevelButton->setEnabled(true);
   });

   CreateButton(openLoadLevelLayout, "Save", [&] {
      auto file = nanogui::file_dialog({{"dgl", "DGame Level"}}, true);
      m_parent.SaveLevel(file);
   });

   CreateLabel(toolsWindow, "Create new Level");
   CreateButton(toolsWindow, "Create", [&, playLevelButton] {
      auto windowCenter = m_parent.GetScreenSize() / glm::ivec2(2.0f);
      auto createLevelWindow = CreateWindow(&m_parent, "CREATE LEVEL", windowCenter, new nanogui::GridLayout());

      CreateLabel(createLevelWindow, "Width");
      auto widthTextBox = CreateTextBox(createLevelWindow);

      CreateLabel(createLevelWindow, "Height");
      auto heightTextBox = CreateTextBox(createLevelWindow);

      CreateButton(createLevelWindow, "Create", [&, createLevelWindow, widthTextBox, heightTextBox, playLevelButton] {
         m_parent.CreateLevel(glm::ivec2(std::stoi(widthTextBox->value()), std::stoi(heightTextBox->value())));
         playLevelButton->setEnabled(true);
         createLevelWindow->dispose();
         m_parent.performLayout();
      });
      CreateButton(createLevelWindow, "Cancel", [&, createLevelWindow] {
         createLevelWindow->dispose();
         m_parent.performLayout();
      });

      m_parent.performLayout();
   });
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
Gui::CreateButton(nanogui::Widget* parent, const std::string& caption, const std::function< void() >& callback, bool enabled)
{
   auto playLevelButton = new nanogui::Button(parent, caption);
   playLevelButton->setCallback(callback);
   playLevelButton->setEnabled(enabled);

   return playLevelButton;
}

nanogui::TextBox*
Gui::CreateTextBox(nanogui::Widget* parent, const glm::ivec2& size, bool editable)
{
   auto textBox = new nanogui::TextBox(parent);
   textBox->setEditable(editable);
   textBox->setFixedSize(nanogui::Vector2i(size.x, size.y));
   textBox->setValue("3000");
   textBox->setDefaultValue("0.0");
   textBox->setFontSize(16);
   textBox->setFormat("[-]?[0-9]*\\.?[0-9]+");

   return textBox;
}