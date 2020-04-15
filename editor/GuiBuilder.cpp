#include "GuiBuilder.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <nanovg.h>
#include <fstream>

nanogui::Widget*
GuiBuilder::CreateLayout(nanogui::Widget* parent, nanogui::Layout* layout)
{
   auto tools = new nanogui::Widget(parent);
   tools->setLayout(layout);

   return tools;
}

nanogui::Window*
GuiBuilder::CreateWindow(nanogui::Widget* parent, const std::string& caption, const glm::ivec2& position, nanogui::Layout* layout)
{
   auto window = new nanogui::Window(parent, caption);
   window->setPosition(Eigen::Vector2i(position.x, position.y));
   window->setLayout(layout);

   return window;
}

nanogui::Label*
GuiBuilder::CreateLabel(nanogui::Widget* parent, const std::string& caption)
{
   return new nanogui::Label(parent, caption, "sans-bold");
}

nanogui::Button*
GuiBuilder::CreateButton(nanogui::Widget* parent, const std::string& caption, const std::function< void() >& callback, int icon,
                         bool enabled)
{
   auto button = new nanogui::Button(parent, caption, icon);
   button->setCallback(callback);
   button->setEnabled(enabled);

   return button;
}

nanogui::TextBox*
GuiBuilder::CreateTextBox(nanogui::Widget* parent, const std::string& value, const glm::ivec2& size, bool editable)
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
GuiBuilder::CreateCheckBox(nanogui::Widget* parent, const std::function< void(bool) >& callback, const std::string& text, float fontSize,
                    bool checked)
{
   auto checkBox = new nanogui::CheckBox(parent, text);
   checkBox->setFontSize(fontSize);
   checkBox->setChecked(checked);
   checkBox->setCallback(callback);

   return checkBox;
}

std::pair< nanogui::PopupButton*, nanogui::Popup* >
GuiBuilder::CreatePopupButton(nanogui::Widget* parent, const std::string& text, nanogui::Layout* layout, int icon, bool enabled)
{
   nanogui::PopupButton* popupBtn = new nanogui::PopupButton(parent, text, icon);
   popupBtn->setEnabled(enabled);

   nanogui::Popup* popup = popupBtn->popup();
   popup->setLayout(layout);

   return {popupBtn, popup};
}
