#pragma once

#include <nanogui/window.h>

class Editor;
class Level;

class LevelWindow : public nanogui::Window
{
 public:
   LevelWindow(Editor& parent);
   ~LevelWindow() = default;

   void
   Update();

   void
   LevelLoaded(Level* loadedLevel);

 private:
   nanogui::TextBox* m_width = nullptr;
   nanogui::TextBox* m_height = nullptr;
   nanogui::Button* m_textureButton = nullptr;
   nanogui::Button* m_shaderButton = nullptr;
   nanogui::CheckBox* m_wireframe = nullptr;
   nanogui::CheckBox* m_showCollision = nullptr;

   Editor& m_parent;
   Level* m_loadedLevel = nullptr;
   bool m_created = false;
};
