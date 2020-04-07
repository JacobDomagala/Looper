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
   // auto toolsWindow = new nanogui::Window(&m_parent, "TOOLS");
   // toolsWindow->setPosition(Eigen::Vector2i(0, 0));
   // toolsWindow->setLayout(new nanogui::GroupLayout());

   ///* No need to store a pointer, the data structure will be automatically
   // freed when the parent window is deleted */

   //// PLAY LEVEL
   // new nanogui::Label(toolsWindow, "Play current level", "sans-bold");
   // auto* playLevelButton = new nanogui::Button(toolsWindow, "Play level");
   // playLevelButton->setCallback([&] { m_parent.PlayLevel(); });
   // playLevelButton->setEnabled(false);

   //// OPEN/SAVE LEVEL
   // new nanogui::Label(toolsWindow, "Open/Save Level", "sans-bold");
   // auto tools = new nanogui::Widget(toolsWindow);
   // tools->setLayout(
   //   new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 6));

   //// open
   // auto openLevelButton = new nanogui::Button(tools, "Open");
   // openLevelButton->setCallback([&] {
   //   auto file = nanogui::file_dialog({{"dgl", "Game level"}}, false);
   //   m_parent.LoadLevel(file);
   //   m_parent.childAt(0)->childAt(1)->setEnabled(true);
   //});

   //// save
   // auto saveLevelButton = new nanogui::Button(tools, "Save");
   // saveLevelButton->setCallback([&] {
   //   auto file = nanogui::file_dialog({{"dgl", "Game level"}}, true);
   //   m_parent.SaveLevel(file);
   //});

   auto toolsWindow =
      CreateWindow(&m_parent, "TOOLS", glm::ivec2(0, 0), new nanogui::GroupLayout());

   CreateLabel(toolsWindow, "Play current level");
   CreateButton(
      toolsWindow, "PlayLevel", [&] { m_parent.PlayLevel(); }, false);

   CreateLabel(toolsWindow, "Open/Save Level");
   auto tools = new nanogui::Widget(toolsWindow);
   tools->setLayout(
      new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 6));

   CreateButton(tools, "Open", [&] {
      auto file = nanogui::file_dialog({{"dgl", "Game level"}}, false);
      m_parent.LoadLevel(file);
      m_parent.childAt(0)->childAt(1)->setEnabled(true);
   });

   CreateButton(tools, "Save", [&] {
      auto file = nanogui::file_dialog({{"dgl", "Game level"}}, true);
      m_parent.SaveLevel(file);
   });
}

nanogui::Window*
Gui::CreateWindow(nanogui::Widget* parent, const std::string& caption, const glm::ivec2& position,
                  nanogui::Layout* layout)
{
   auto* window = new nanogui::Window(parent, caption);
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
Gui::CreateButton(nanogui::Widget* parent, const std::string& caption,
                  const std::function< void() >& callback, bool enabled)

{
   auto* playLevelButton = new nanogui::Button(parent, caption);
   playLevelButton->setCallback(callback);
   playLevelButton->setEnabled(enabled);

   return playLevelButton;
}