#pragma once

#include "Section.hpp"
#include "GuiAPI.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>

namespace dgame {

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
   static Widget*
   CreateLayout(Widget* parent, LayoutType type, Orientation orientation = Orientation::Horizontal, int resolution = 2,
                Alignment alignment = Alignment::Middle, int marigin = 0, int spacing = 6);

   static GuiWindow*
   CreateWindow(Widget* parent, const std::string& caption, const glm::ivec2& position, Layout* layout);

   static ImageView*
   CreateImageView(Widget* parent, GLuint textureID, const glm::ivec2& size);

   static Slider*
   CreateSlider(Widget* parent, const std::function< void(float) >& dragCallback, std::pair< float, float > range, float value,
                int fixedWidth = 0);

   static Widget*
   CreateBlankSpace(Widget* parent);

   static Label*
   CreateLabel(Widget* parent, const std::string& caption);

   static Label*
   CreateTitleLabel(Widget* parent, const std::string& caption);

   static Button*
   CreateButton(Widget* parent, const std::string& caption, const std::function< void() >& callback, int icon = 0, int fixedWidth = 0,
                bool enabled = true);

   static Button*
   CreateRadioButton(Widget* parent, const std::string& caption, const std::function< void() >& callback, int icon = 0, int fixedWidth = 0,
                     bool enabled = true);

   static TextBox*
   CreateTextBox(
      Widget* parent, const std::string& value,
      const std::function< bool(const std::string&) >& callback = [](const std::string&) { return true; }, bool numeric = true,
      const glm::ivec2& size = glm::ivec2(0, 0), bool editable = false);

   template < typename T >
   static IntBox< T >*
   CreateNumericBox(
      Widget* parent, const T& value, std::pair< T, T > range = {T(), T()},
      const std::function< bool(const T&) >& callback = [](const T&) { return true; }, const glm::ivec2& size = glm::ivec2(0, 0),
      bool editable = true);

   static TextBox*
   CreateFloatingPointBox(
      Widget* parent, float value, std::pair< float, float > range = {0.0f, 1.0f},
      const std::function< bool(const std::string&) >& callback = [](const std::string&) { return true; },
      const glm::ivec2& size = glm::ivec2(0, 0), bool editable = true);

   static CheckBox*
   CreateCheckBox(Widget* parent, const std::function< void(bool) >& callback, const std::string& text = "", float fontSize = 16,
                  bool checked = false);

   static std::pair< PopupBtn*, Popup* >
   CreatePopupButton(Widget* parent, const std::string& text, Popup::Side side = Popup::Side::Right, Layout* = new GridLayout, int icon = 0,
                     bool enabled = true);
};

} // namespace dgame