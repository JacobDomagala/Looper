#include <GL/glew.h>
#include <nanogui/nanogui.h>

namespace dgame {

using GuiObject = ::nanogui::Object;
using GuiWindow = ::nanogui::Window;
using Widget = nanogui::Widget;
using Orientation = nanogui::Orientation;
using Alignment = nanogui::Alignment;
using Layout = nanogui::Layout;
using GridLayout = nanogui::GridLayout;
using BoxLayout = nanogui::BoxLayout;
using GroupLayout  = nanogui::GroupLayout ;
using ImageView = nanogui::ImageView;
using Slider = nanogui::Slider;
using Label = nanogui::Label;
using TextBox = nanogui::TextBox;
using CheckBox = nanogui::CheckBox;
using PopupBtn = ::nanogui::PopupButton;
using Popup = nanogui::Popup;
using Button = nanogui::Button;
using VscrollPanel = nanogui::VScrollPanel;

template < typename T > using IntBox = nanogui::IntBox< T >;

} // namespace dgame