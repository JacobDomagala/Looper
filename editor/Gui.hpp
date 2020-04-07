#pragma once

#include <glm/glm.hpp>
#include <nanogui/button.h>
#include <nanogui/label.h>
#include <nanogui/layout.h>
#include <nanogui/window.h>
#include <nanogui/textbox.h>

class Editor;

class Gui
{
 public:
   Gui(Editor& parent);
   ~Gui() = default;

   void
   Init();

 private:
   /*
    Creates left panel with following subwidgets:
    - Play currently loaded level
    - Load/Save current level
    */
   void
   CreateLeftPanel();

   nanogui::Widget*
   CreateLayout(nanogui::Widget* parent, nanogui::Layout* layout);

   nanogui::Window*
   CreateWindow(nanogui::Widget* parent, const std::string& caption, const glm::ivec2& position, nanogui::Layout* layout);

   nanogui::Label*
   CreateLabel(nanogui::Widget* parent, const std::string& caption);

   nanogui::Button*
   CreateButton(nanogui::Widget* parent, const std::string& caption, const std::function< void() >& callback, bool enabled = true);

   nanogui::TextBox*
   CreateTextBox(nanogui::Widget* parent, const glm::ivec2& size = glm::ivec2(100, 20), bool editable = true);

 private:
   Editor& m_parent;
   // std::unordered_map< std::string, nanogui::Window > m_windows;
};
