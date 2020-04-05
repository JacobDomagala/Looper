#include "Gui.hpp"
#include "Editor.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <nanogui/button.h>
#include <nanogui/label.h>
#include <nanogui/layout.h>
#include <nanogui/window.h>
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
   auto toolsWindow = new nanogui::Window(&m_parent, "TOOLS");
   toolsWindow->setPosition(Eigen::Vector2i(0, 0));
   toolsWindow->setLayout(new nanogui::GroupLayout());

   /* No need to store a pointer, the data structure will be automatically
    freed when the parent window is deleted */

   // PLAY LEVEL
   new nanogui::Label(toolsWindow, "Play current level", "sans-bold");
   auto* playLevelButton = new nanogui::Button(toolsWindow, "Play level");
   playLevelButton->setCallback([&] { m_parent.PlayLevel(); });
   playLevelButton->setEnabled(false);

   // OPEN/SAVE LEVEL
   new nanogui::Label(toolsWindow, "Open/Save Level", "sans-bold");
   auto tools = new nanogui::Widget(toolsWindow);
   tools->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle, 0, 6));

   auto openLevelButton = new nanogui::Button(tools, "Open");
   openLevelButton->setCallback([&] {
      auto file = nanogui::file_dialog({{"dgl", "Game level"}}, false);
      m_parent.LoadLevel(file);
      m_parent.childAt(0)->childAt(1)->setEnabled(true);
   });

   auto saveLevelButton = new nanogui::Button(tools, "Save");
   saveLevelButton->setCallback([&] { auto file = nanogui::file_dialog({{"dgl", "Game level"}}, false); });
}