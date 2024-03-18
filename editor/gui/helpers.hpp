#include <GLFW/glfw3.h>
#include <imgui.h>

#include <string_view>

namespace looper {

static inline ImGuiKey
KeyToImGuiKey(int key)
{
   switch (key)
   {
      case GLFW_KEY_TAB:
         return ImGuiKey_Tab;
      case GLFW_KEY_LEFT:
         return ImGuiKey_LeftArrow;
      case GLFW_KEY_RIGHT:
         return ImGuiKey_RightArrow;
      case GLFW_KEY_UP:
         return ImGuiKey_UpArrow;
      case GLFW_KEY_DOWN:
         return ImGuiKey_DownArrow;
      case GLFW_KEY_PAGE_UP:
         return ImGuiKey_PageUp;
      case GLFW_KEY_PAGE_DOWN:
         return ImGuiKey_PageDown;
      case GLFW_KEY_HOME:
         return ImGuiKey_Home;
      case GLFW_KEY_END:
         return ImGuiKey_End;
      case GLFW_KEY_INSERT:
         return ImGuiKey_Insert;
      case GLFW_KEY_DELETE:
         return ImGuiKey_Delete;
      case GLFW_KEY_BACKSPACE:
         return ImGuiKey_Backspace;
      case GLFW_KEY_SPACE:
         return ImGuiKey_Space;
      case GLFW_KEY_ENTER:
         return ImGuiKey_Enter;
      case GLFW_KEY_ESCAPE:
         return ImGuiKey_Escape;
      case GLFW_KEY_APOSTROPHE:
         return ImGuiKey_Apostrophe;
      case GLFW_KEY_COMMA:
         return ImGuiKey_Comma;
      case GLFW_KEY_MINUS:
         return ImGuiKey_Minus;
      case GLFW_KEY_PERIOD:
         return ImGuiKey_Period;
      case GLFW_KEY_SLASH:
         return ImGuiKey_Slash;
      case GLFW_KEY_SEMICOLON:
         return ImGuiKey_Semicolon;
      case GLFW_KEY_EQUAL:
         return ImGuiKey_Equal;
      case GLFW_KEY_LEFT_BRACKET:
         return ImGuiKey_LeftBracket;
      case GLFW_KEY_BACKSLASH:
         return ImGuiKey_Backslash;
      case GLFW_KEY_RIGHT_BRACKET:
         return ImGuiKey_RightBracket;
      case GLFW_KEY_GRAVE_ACCENT:
         return ImGuiKey_GraveAccent;
      case GLFW_KEY_CAPS_LOCK:
         return ImGuiKey_CapsLock;
      case GLFW_KEY_SCROLL_LOCK:
         return ImGuiKey_ScrollLock;
      case GLFW_KEY_NUM_LOCK:
         return ImGuiKey_NumLock;
      case GLFW_KEY_PRINT_SCREEN:
         return ImGuiKey_PrintScreen;
      case GLFW_KEY_PAUSE:
         return ImGuiKey_Pause;
      case GLFW_KEY_KP_0:
         return ImGuiKey_Keypad0;
      case GLFW_KEY_KP_1:
         return ImGuiKey_Keypad1;
      case GLFW_KEY_KP_2:
         return ImGuiKey_Keypad2;
      case GLFW_KEY_KP_3:
         return ImGuiKey_Keypad3;
      case GLFW_KEY_KP_4:
         return ImGuiKey_Keypad4;
      case GLFW_KEY_KP_5:
         return ImGuiKey_Keypad5;
      case GLFW_KEY_KP_6:
         return ImGuiKey_Keypad6;
      case GLFW_KEY_KP_7:
         return ImGuiKey_Keypad7;
      case GLFW_KEY_KP_8:
         return ImGuiKey_Keypad8;
      case GLFW_KEY_KP_9:
         return ImGuiKey_Keypad9;
      case GLFW_KEY_KP_DECIMAL:
         return ImGuiKey_KeypadDecimal;
      case GLFW_KEY_KP_DIVIDE:
         return ImGuiKey_KeypadDivide;
      case GLFW_KEY_KP_MULTIPLY:
         return ImGuiKey_KeypadMultiply;
      case GLFW_KEY_KP_SUBTRACT:
         return ImGuiKey_KeypadSubtract;
      case GLFW_KEY_KP_ADD:
         return ImGuiKey_KeypadAdd;
      case GLFW_KEY_KP_ENTER:
         return ImGuiKey_KeypadEnter;
      case GLFW_KEY_KP_EQUAL:
         return ImGuiKey_KeypadEqual;
      case GLFW_KEY_LEFT_SHIFT:
         return ImGuiKey_LeftShift;
      case GLFW_KEY_LEFT_CONTROL:
         return ImGuiKey_LeftCtrl;
      case GLFW_KEY_LEFT_ALT:
         return ImGuiKey_LeftAlt;
      case GLFW_KEY_LEFT_SUPER:
         return ImGuiKey_LeftSuper;
      case GLFW_KEY_RIGHT_SHIFT:
         return ImGuiKey_RightShift;
      case GLFW_KEY_RIGHT_CONTROL:
         return ImGuiKey_RightCtrl;
      case GLFW_KEY_RIGHT_ALT:
         return ImGuiKey_RightAlt;
      case GLFW_KEY_RIGHT_SUPER:
         return ImGuiKey_RightSuper;
      case GLFW_KEY_MENU:
         return ImGuiKey_Menu;
      case GLFW_KEY_0:
         return ImGuiKey_0;
      case GLFW_KEY_1:
         return ImGuiKey_1;
      case GLFW_KEY_2:
         return ImGuiKey_2;
      case GLFW_KEY_3:
         return ImGuiKey_3;
      case GLFW_KEY_4:
         return ImGuiKey_4;
      case GLFW_KEY_5:
         return ImGuiKey_5;
      case GLFW_KEY_6:
         return ImGuiKey_6;
      case GLFW_KEY_7:
         return ImGuiKey_7;
      case GLFW_KEY_8:
         return ImGuiKey_8;
      case GLFW_KEY_9:
         return ImGuiKey_9;
      case GLFW_KEY_A:
         return ImGuiKey_A;
      case GLFW_KEY_B:
         return ImGuiKey_B;
      case GLFW_KEY_C:
         return ImGuiKey_C;
      case GLFW_KEY_D:
         return ImGuiKey_D;
      case GLFW_KEY_E:
         return ImGuiKey_E;
      case GLFW_KEY_F:
         return ImGuiKey_F;
      case GLFW_KEY_G:
         return ImGuiKey_G;
      case GLFW_KEY_H:
         return ImGuiKey_H;
      case GLFW_KEY_I:
         return ImGuiKey_I;
      case GLFW_KEY_J:
         return ImGuiKey_J;
      case GLFW_KEY_K:
         return ImGuiKey_K;
      case GLFW_KEY_L:
         return ImGuiKey_L;
      case GLFW_KEY_M:
         return ImGuiKey_M;
      case GLFW_KEY_N:
         return ImGuiKey_N;
      case GLFW_KEY_O:
         return ImGuiKey_O;
      case GLFW_KEY_P:
         return ImGuiKey_P;
      case GLFW_KEY_Q:
         return ImGuiKey_Q;
      case GLFW_KEY_R:
         return ImGuiKey_R;
      case GLFW_KEY_S:
         return ImGuiKey_S;
      case GLFW_KEY_T:
         return ImGuiKey_T;
      case GLFW_KEY_U:
         return ImGuiKey_U;
      case GLFW_KEY_V:
         return ImGuiKey_V;
      case GLFW_KEY_W:
         return ImGuiKey_W;
      case GLFW_KEY_X:
         return ImGuiKey_X;
      case GLFW_KEY_Y:
         return ImGuiKey_Y;
      case GLFW_KEY_Z:
         return ImGuiKey_Z;
      case GLFW_KEY_F1:
         return ImGuiKey_F1;
      case GLFW_KEY_F2:
         return ImGuiKey_F2;
      case GLFW_KEY_F3:
         return ImGuiKey_F3;
      case GLFW_KEY_F4:
         return ImGuiKey_F4;
      case GLFW_KEY_F5:
         return ImGuiKey_F5;
      case GLFW_KEY_F6:
         return ImGuiKey_F6;
      case GLFW_KEY_F7:
         return ImGuiKey_F7;
      case GLFW_KEY_F8:
         return ImGuiKey_F8;
      case GLFW_KEY_F9:
         return ImGuiKey_F9;
      case GLFW_KEY_F10:
         return ImGuiKey_F10;
      case GLFW_KEY_F11:
         return ImGuiKey_F11;
      case GLFW_KEY_F12:
         return ImGuiKey_F12;
      default:
         return ImGuiKey_None;
   }
}

static inline void
SetStyle()
{
   // NOLINTNEXTLINE
   ImVec4* colors = ImGui::GetStyle().Colors;

   colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
   colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
   colors[ImGuiCol_WindowBg] = ImVec4(0.01f, 0.01f, 0.01f, 0.99f);
   colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
   colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
   colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
   colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
   colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
   colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
   colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
   colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
   colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
   colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
   colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
   colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
   colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
   colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
   colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
   colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
   colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
   colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
   colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
   colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
   colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
   colors[ImGuiCol_Header] = ImVec4(0.3f, 0.5f, 0.1f, 0.52f);
   colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.30f, 0.00f, 0.36f);
   colors[ImGuiCol_HeaderActive] = ImVec4(0.3f, 0.5f, 0.1f, 0.33f);
   colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
   colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
   colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
   colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
   colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
   colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
   colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
   colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
   colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
   colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
   colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);

#ifdef IMGUI_HAS_DOCK
   colors[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
   colors[ImGuiCol_DockingEmptyBg] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
#endif

   colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
   colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
   colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
   colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
   colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
   colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
   colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
   colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
   colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
   colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
   colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
   colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
   colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
   colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
   colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

   ImGuiStyle& style = ImGui::GetStyle();
   style.WindowPadding = ImVec2(8.00f, 8.00f);
   style.FramePadding = ImVec2(5.00f, 2.00f);
   style.CellPadding = ImVec2(2.00f, 2.00f);
   style.ItemSpacing = ImVec2(5.00f, 5.00f);
   style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
   style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
   style.IndentSpacing = 25;
   style.ScrollbarSize = 15;
   style.GrabMinSize = 10;
   style.WindowBorderSize = 1;
   style.ChildBorderSize = 1;
   style.PopupBorderSize = 1;
   style.FrameBorderSize = 1;
   style.TabBorderSize = 1;
   style.WindowRounding = 7;
   style.ChildRounding = 4;
   style.FrameRounding = 3;
   style.PopupRounding = 4;
   style.ScrollbarRounding = 9;
   style.GrabRounding = 3;
   style.LogSliderDeadzone = 4;
   style.TabRounding = 4;
}

static inline void
BlankLine(const ImVec2& line = ImVec2(0.0f, 5.0f))
{
   ImGui::Dummy(line);
}

template < typename Action >
static constexpr inline void
DrawWidget(std::string_view label, const Action& action)
{
   ImGui::Text("%s", label.data());
   ImGui::PushItemWidth(-1);

   action();

   ImGui::PopItemWidth();
   ImGui::Spacing();
}

static inline void
CreateRow(std::string_view name, std::string_view value)
{
   ImGui::TableNextRow();
   ImGui::TableNextColumn();
   ImGui::Text("%s", name.data());
   ImGui::TableNextColumn();
   ImGui::Text("%s", value.data());
}

static inline void 
BlankColumn()
{
   ImGui::TableNextColumn();
   BlankLine();
}

static inline void 
FillWidth()
{
   ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
}

template < typename Action >
static inline void
CreateActionColumn(const Action& firstAction)
{
   ImGui::TableNextColumn();
   firstAction();
}

template < typename FirstAction, typename... Actions >
static inline void
CreateActionRowLabel(std::string_view name, const FirstAction& firstAction, const Actions&... actions)
{
   ImGui::TableNextRow();
   ImGui::TableNextColumn();
   ImGui::Text("%s", name.data());
   CreateActionColumn(firstAction);

   (CreateActionColumn(actions), ...);
}

template < typename FirstAction, typename... Actions >
static inline void
CreateActionRow(const FirstAction& firstAction, const Actions&... actions)
{
   ImGui::TableNextRow();
   CreateActionColumn(firstAction);

   (CreateActionColumn(actions), ...);
}

} // namespace looper
