#pragma once

#include "Section.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <nanogui/button.h>
#include <nanogui/checkbox.h>
#include <nanogui/label.h>
#include <nanogui/layout.h>
#include <nanogui/popupbutton.h>
#include <nanogui/slider.h>
#include <nanogui/textbox.h>

class Sprite;
class GuiBuilder
{
 public:
   enum class LayoutType
   {
      GRID,
      BOX,
      GROUP
   };

 public:
   static nanogui::Widget*
   CreateLayout(nanogui::Widget* parent, LayoutType type, nanogui::Orientation orientation = nanogui::Orientation::Horizontal,
                int resolution = 2, nanogui::Alignment alignment = nanogui::Alignment::Middle, int marigin = 0, int spacing = 6);

   static nanogui::Window*
   CreateWindow(nanogui::Widget* parent, const std::string& caption, const glm::ivec2& position, nanogui::Layout* layout);

   static nanogui::ImageView*
   CreateImageView(nanogui::Widget* parent, GLuint textureID, const glm::ivec2& size);

   static nanogui::Slider*
   CreateSlider(nanogui::Widget* parent, const std::function< void(float) >& dragCallback, std::pair< float, float > range, float value,
                int fixedWidth = 0);

   static nanogui::Widget*
   CreateBlankSpace(nanogui::Widget* parent);

   static nanogui::Label*
   CreateLabel(nanogui::Widget* parent, const std::string& caption);

   static nanogui::Label*
   CreateTitleLabel(nanogui::Widget* parent, const std::string& caption);

   static nanogui::Button*
   CreateButton(nanogui::Widget* parent, const std::string& caption, const std::function< void() >& callback, int icon = 0,
                int fixedWidth = 0, bool enabled = true);

   static nanogui::Button*
   CreateRadioButton(nanogui::Widget* parent, const std::string& caption, const std::function< void() >& callback, int icon = 0,
                     int fixedWidth = 0, bool enabled = true);

   static nanogui::TextBox*
   CreateTextBox(
      nanogui::Widget* parent, const std::string& value,
      const std::function< bool(const std::string&) >& callback = [](const std::string&) { return true; }, bool numeric = true,
      const glm::ivec2& size = glm::ivec2(0, 0), bool editable = false);

   template < typename T >
   static nanogui::IntBox< T >*
   CreateNumericBox(
      nanogui::Widget* parent, const T& value, std::pair< T, T > range = {T(), T()},
      const std::function< bool(const T&) >& callback = [](const T&) { return true; }, const glm::ivec2& size = glm::ivec2(0, 0),
      bool editable = true);

   static nanogui::TextBox*
   CreateFloatingPointBox(
      nanogui::Widget* parent, float value, std::pair< float, float > range = {0.0f, 1.0f},
      const std::function< bool(const std::string&) >& callback = [](const std::string&) { return true; },
      const glm::ivec2& size = glm::ivec2(0, 0), bool editable = true);

   static nanogui::CheckBox*
   CreateCheckBox(nanogui::Widget* parent, const std::function< void(bool) >& callback, const std::string& text = "", float fontSize = 16,
                  bool checked = false);

   static std::pair< nanogui::PopupButton*, nanogui::Popup* >
   CreatePopupButton(nanogui::Widget* parent, const std::string& text, nanogui::Layout* = new nanogui::GridLayout, int icon = 0,
                     bool enabled = true);

   static Section*
   CreateSection(nanogui::Widget* parent, const std::string& name, bool active = true);
};