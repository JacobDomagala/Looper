#pragma once

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

 private:
   Editor& m_parent;
   // std::unordered_map< std::string, nanogui::Window > m_windows;
};
