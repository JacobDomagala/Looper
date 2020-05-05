#include "GuiBuilder.hpp"
#include "Utils.hpp"

#include <nanogui/imageview.h>

namespace dgame {

Widget*
GuiBuilder::CreateLayout(Widget* parent, LayoutType type, Orientation orientation, int resolution, Alignment alignment, int marigin,
                         int spacing)
{
   auto tools = new Widget(parent);
   Layout* layout = nullptr;

   switch (type)
   {
      case LayoutType::BOX: {
         layout = new BoxLayout(orientation);
      }
      break;

      case LayoutType::GRID: {
         layout = new GridLayout(orientation, resolution, alignment, marigin, spacing);
      }
      break;

      case LayoutType::GROUP: {
         layout = new BoxLayout(orientation, alignment, marigin, spacing);
      }
      break;
   }


   tools->setLayout(layout);

   return tools;
}

GuiWindow*
GuiBuilder::CreateWindow(Widget* parent, const std::string& caption, const glm::ivec2& position, Layout* layout)
{
   auto window = new GuiWindow(parent, caption);
   window->setPosition(Eigen::Vector2i(position.x, position.y));
   window->setLayout(layout);

   return window;
}

ImageView*
GuiBuilder::CreateImageView(Widget* parent, GLuint textureID, const glm::ivec2& size)
{
   auto imageView = new ImageView(parent, textureID);

   imageView->setFixedSize({size.x, size.y});
   imageView->setSize({size.x, size.y});
   imageView->bindImage(textureID);

   return imageView;
}

Slider*
GuiBuilder::CreateSlider(Widget* parent, const std::function< void(float) >& callback, std::pair< float, float > range, float value,
                         int fixedWidth)
{
   Slider* slider = new Slider(parent);
   slider->setRange(range);
   slider->setValue(value);
   slider->setFixedWidth(fixedWidth);
   slider->setCallback(callback);

   return slider;
}

Widget*
GuiBuilder::CreateBlankSpace(Widget* parent)
{
   return GuiBuilder::CreateLabel(parent, " ");
}

Label*
GuiBuilder::CreateLabel(Widget* parent, const std::string& caption)
{
   return new Label(parent, caption, "sans-bold");
}

Label*
GuiBuilder::CreateTitleLabel(Widget* parent, const std::string& caption)
{
   return GuiBuilder::CreateLabel(GuiBuilder::CreateLayout(parent, GuiBuilder::LayoutType::GRID, Orientation::Horizontal, 1), caption);
}

Button*
GuiBuilder::CreateButton(Widget* parent, const std::string& caption, const std::function< void() >& callback, int icon, int fixedWidth,
                         bool enabled)
{
   auto button = new Button(parent, caption, icon);
   button->setCallback(callback);
   button->setEnabled(enabled);
   button->setFixedWidth(fixedWidth);

   return button;
}

Button*
GuiBuilder::CreateRadioButton(Widget* parent, const std::string& caption, const std::function< void() >& callback, int icon, int fixedWidth,
                              bool enabled)
{
   auto button = GuiBuilder::CreateButton(parent, caption, callback, icon, fixedWidth, enabled);
   button->setFlags(Button::RadioButton);

   return button;
}

TextBox*
GuiBuilder::CreateTextBox(Widget* parent, const std::string& value, const std::function< bool(const std::string&) >& callback, bool numeric,
                          const glm::ivec2& size, bool editable)
{
   auto textBox = new TextBox(parent);
   textBox->setEditable(editable);
   textBox->setFixedSize({size.x, size.y});
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
static IntBox< T >*
GuiBuilder::CreateNumericBox(Widget* parent, const T& value, std::pair< T, T > range, const std::function< bool(const T&) >& callback,
                             const glm::ivec2& size, bool editable)
{
   auto numericBox = new IntBox< T >(parent);
   numericBox->setEditable(editable);
   numericBox->setFixedSize({size.x, size.y});
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

TextBox*
GuiBuilder::CreateFloatingPointBox(Widget* parent, float value, std::pair< float, float > range,
                                   const std::function< bool(const std::string&) >& callback, const glm::ivec2& size, bool editable)
{
   auto textBox = new TextBox(parent);
   textBox->setEditable(editable);
   textBox->setFixedSize({size.x, size.y});


   textBox->setValue(CustomFloatToStr(value));
   textBox->setFontSize(16);

   textBox->setFormat("[-]?[0-9]*\\.?[0-9]+");

   textBox->setCallback(callback);

   return textBox;
}


CheckBox*
GuiBuilder::CreateCheckBox(Widget* parent, const std::function< void(bool) >& callback, const std::string& text, float fontSize,
                           bool checked)
{
   auto checkBox = new CheckBox(parent, text);
   checkBox->setFontSize(fontSize);
   checkBox->setChecked(checked);
   checkBox->setCallback(callback);

   return checkBox;
}

std::pair< PopupButton*, Popup* >
GuiBuilder::CreatePopupButton(Widget* parent, const std::string& text, Popup::Side side, Layout* layout, int icon, bool enabled)
{
   PopupButton* popupBtn = new PopupButton(parent, text, icon);
   popupBtn->setEnabled(enabled);
   popupBtn->setSide(side);

   Popup* popup = popupBtn->popup();
   popup->setLayout(layout);
   popup->setSide(side);

   return {popupBtn, popup};
}

template IntBox< uint32_t >*
GuiBuilder::CreateNumericBox< uint32_t >(Widget* parent, const uint32_t& value, std::pair< uint32_t, uint32_t > range,
                                         const std::function< bool(const uint32_t&) >& callback, const glm::ivec2& size, bool editable);

template IntBox< int32_t >*
GuiBuilder::CreateNumericBox< int32_t >(Widget* parent, const int32_t& value, std::pair< int32_t, int32_t > range,
                                        const std::function< bool(const int32_t&) >& callback, const glm::ivec2& size, bool editable);

} // namespace dgame