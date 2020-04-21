#pragma once

#include <nanogui/window.h>
#include <functional>

class Editor;
class Level;
class Section;

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
   void
   CreateGeneralSection();

   void
   CreateShaderSection();

 private:
   nanogui::TextBox* m_width = nullptr;
   nanogui::TextBox* m_height = nullptr;
   nanogui::Button* m_textureButton = nullptr;
   nanogui::Button* m_shaderButton = nullptr;
   nanogui::CheckBox* m_wireframe = nullptr;
   nanogui::CheckBox* m_showCollision = nullptr;

   Section* m_generalSection = nullptr;
   Section* m_shaderSection = nullptr;

   Editor& m_parent;
   
   bool m_created = false;
   
   Level* m_loadedLevel = nullptr;
   std::function< void() > m_textureChangeCallback;
};
