#pragma once

#include <glm/glm.hpp>
#include <nanogui/button.h>
#include <nanogui/checkbox.h>
#include <nanogui/label.h>
#include <nanogui/layout.h>
#include <nanogui/popupbutton.h>
#include <nanogui/textbox.h>

class GuiBuilder
{
 public:
   static nanogui::Widget*
   CreateLayout(nanogui::Widget* parent, nanogui::Layout* layout);

   static nanogui::Window*
   CreateWindow(nanogui::Widget* parent, const std::string& caption, const glm::ivec2& position, nanogui::Layout* layout);

   static nanogui::Label*
   CreateLabel(nanogui::Widget* parent, const std::string& caption);

   static nanogui::Button*
   CreateButton(nanogui::Widget* parent, const std::string& caption, const std::function< void() >& callback, int icon = 0,
                bool enabled = true);

   static nanogui::TextBox*
   CreateTextBox(nanogui::Widget* parent, const std::string& value = "dummyValue", const glm::ivec2& size = glm::ivec2(100, 20),
                 bool editable = true);

   static nanogui::CheckBox*
   CreateCheckBox(nanogui::Widget* parent, const std::function< void(bool) >& callback, const std::string& text = "", float fontSize = 16,
                  bool checked = false);

   static std::pair< nanogui::PopupButton*, nanogui::Popup* >
   CreatePopupButton(nanogui::Widget* parent, const std::string& text, nanogui::Layout* = new nanogui::GridLayout, int icon = 0,
                     bool enabled = true);
};
