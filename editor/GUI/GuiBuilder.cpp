#include "GuiBuilder.hpp"
#include "Utils.hpp"

#include <nanogui/imageview.h>

nanogui::Widget*
GuiBuilder::CreateLayout(nanogui::Widget* parent, LayoutType type, nanogui::Orientation orientation, int resolution,
                         nanogui::Alignment alignment, int marigin, int spacing)
{
   auto tools = new nanogui::Widget(parent);
   nanogui::Layout* layout = nullptr;

   switch (type)
   {
      case LayoutType::BOX: {
         layout = new nanogui::BoxLayout(orientation);
      }
      break;

      case LayoutType::GRID: {
         layout = new nanogui::GridLayout(orientation, resolution, alignment, marigin, spacing);
      }
      break;

      case LayoutType::GROUP: {
         layout = new nanogui::BoxLayout(orientation, alignment, marigin, spacing);
      }
      break;
   }


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

nanogui::ImageView*
GuiBuilder::CreateImageView(nanogui::Widget* parent, GLuint textureID, const glm::ivec2& size)
{
   auto imageView = new nanogui::ImageView(parent, textureID);

   imageView->setFixedSize({size.x, size.y});
   imageView->setSize({size.x, size.y});
   imageView->bindImage(textureID);

   return imageView;
}

nanogui::Slider*
GuiBuilder::CreateSlider(nanogui::Widget* parent, const std::function< void(float) >& callback, std::pair< float, float > range,
                         float value, int fixedWidth)
{
   nanogui::Slider* slider = new nanogui::Slider(parent);
   slider->setRange(range);
   slider->setValue(value);
   slider->setFixedWidth(fixedWidth);
   slider->setCallback(callback);

   return slider;
}

nanogui::Widget*
GuiBuilder::CreateBlankSpace(nanogui::Widget* parent)
{
   return GuiBuilder::CreateLabel(parent, " ");
}

nanogui::Label*
GuiBuilder::CreateLabel(nanogui::Widget* parent, const std::string& caption)
{
   return new nanogui::Label(parent, caption, "sans-bold");
}

nanogui::Label*
GuiBuilder::CreateTitleLabel(nanogui::Widget* parent, const std::string& caption)
{
   return GuiBuilder::CreateLabel(GuiBuilder::CreateLayout(parent, GuiBuilder::LayoutType::GRID, nanogui::Orientation::Horizontal, 1),
                                  caption);
}

nanogui::Button*
GuiBuilder::CreateButton(nanogui::Widget* parent, const std::string& caption, const std::function< void() >& callback, int icon,
                         int fixedWidth, bool enabled)
{
   auto button = new nanogui::Button(parent, caption, icon);
   button->setCallback(callback);
   button->setEnabled(enabled);
   button->setFixedWidth(fixedWidth);

   return button;
}

nanogui::Button*
GuiBuilder::CreateRadioButton(nanogui::Widget* parent, const std::string& caption, const std::function< void() >& callback, int icon,
                              int fixedWidth, bool enabled)
{
   auto button = GuiBuilder::CreateButton(parent, caption, callback, icon, fixedWidth, enabled);
   button->setFlags(nanogui::Button::RadioButton);

   return button;
}

nanogui::TextBox*
GuiBuilder::CreateTextBox(nanogui::Widget* parent, const std::string& value, const std::function< bool(const std::string&) >& callback,
                          bool numeric, const glm::ivec2& size, bool editable)
{
   auto textBox = new nanogui::TextBox(parent);
   textBox->setEditable(editable);
   textBox->setFixedSize(nanogui::Vector2i(size.x, size.y));
   textBox->setValue(value);
   textBox->setFontSize(16);
   if (numeric)
   {
      textBox->setFormat("[-]?[0-9]*\\.?[0-9]+");
   }
   textBox->setCallback(callback);

   return textBox;
}

template < typename T >
static nanogui::IntBox< T >*
GuiBuilder::CreateNumericBox(nanogui::Widget* parent, const T& value, std::pair< T, T > range,
                             const std::function< bool(const T&) >& callback, const glm::ivec2& size, bool editable)
{
   auto numericBox = new nanogui::IntBox< T >(parent);
   numericBox->setEditable(editable);
   numericBox->setFixedSize(nanogui::Vector2i(size.x, size.y));
   numericBox->setValue(value);
   numericBox->setDefaultValue("0");
   numericBox->setFontSize(16);
   range.first < 0 ? numericBox->setFormat("[-]?[0-9]*\\.?[0-9]+") : numericBox->setFormat("[1-9][0-9]*");
   numericBox->setSpinnable(true);
   numericBox->setMinValue(range.first);
   numericBox->setMaxValue(range.second);
   numericBox->setValueIncrement(2);
   numericBox->setCallback(callback);

   return numericBox;
}

nanogui::TextBox*
GuiBuilder::CreateFloatingPointBox(nanogui::Widget* parent, float value, std::pair< float, float > range,
                                   const std::function< bool(const std::string&) >& callback, const glm::ivec2& size, bool editable)
{
   auto textBox = new nanogui::TextBox(parent);
   textBox->setEditable(editable);
   textBox->setFixedSize(nanogui::Vector2i(size.x, size.y));


   textBox->setValue(CustomFloatToStr(value));
   textBox->setFontSize(16);

   textBox->setFormat("[-]?[0-9]*\\.?[0-9]+");

   textBox->setCallback(callback);

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
GuiBuilder::CreatePopupButton(nanogui::Widget* parent, const std::string& text, nanogui::Popup::Side side, nanogui::Layout* layout, int icon,
                              bool enabled)
{
   nanogui::PopupButton* popupBtn = new nanogui::PopupButton(parent, text, icon);
   popupBtn->setEnabled(enabled);
   popupBtn->setSide(side);

   nanogui::Popup* popup = popupBtn->popup();
   popup->setLayout(layout);
   popup->setSide(side);

   return {popupBtn, popup};
}

template nanogui::IntBox< uint32_t >*
GuiBuilder::CreateNumericBox< uint32_t >(nanogui::Widget* parent, const uint32_t& value, std::pair< uint32_t, uint32_t > range,
                                         const std::function< bool(const uint32_t&) >& callback, const glm::ivec2& size, bool editable);

template nanogui::IntBox< int32_t >*
GuiBuilder::CreateNumericBox< int32_t >(nanogui::Widget* parent, const int32_t& value, std::pair< int32_t, int32_t > range,
                                        const std::function< bool(const int32_t&) >& callback, const glm::ivec2& size, bool editable);