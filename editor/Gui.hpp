#pragma once

#include <glm/glm.hpp>
#include <nanogui/button.h>
#include <nanogui/checkbox.h>
#include <nanogui/label.h>
#include <nanogui/layout.h>
#include <nanogui/textbox.h>
#include <nanogui/window.h>

class Editor;
class GameObject;

class Gui
{
 public:
   Gui(Editor& parent);
   ~Gui() = default;

   void
   Init();

   void
   GameObjectSelected(std::shared_ptr<GameObject> selectedGameObject);

   void
   GameObjectUnselected();

 private:
   /*
    Creates left panel with following subwidgets:
    - Play currently loaded level
    - Load/Save current level
    */
   void
   CreateLeftPanel();

   nanogui::Widget*
   CreateLayout(nanogui::Widget* parent, nanogui::Layout* layout);

   nanogui::Window*
   CreateWindow(nanogui::Widget* parent, const std::string& caption, const glm::ivec2& position, nanogui::Layout* layout);

   nanogui::Label*
   CreateLabel(nanogui::Widget* parent, const std::string& caption);

   nanogui::Button*
   CreateButton(nanogui::Widget* parent, const std::string& caption, const std::function< void() >& callback, bool enabled = true);

   nanogui::TextBox*
   CreateTextBox(nanogui::Widget* parent, const std::string& value = "dummyValue" , const glm::ivec2& size = glm::ivec2(100, 20),
                 bool editable = true);

   nanogui::CheckBox*
   CreateCheckBox(nanogui::Widget* parent, const std::function< void(bool) >& callback, const std::string& text = "", float fontSize = 16,
                  bool checked = false);

 private:
   struct GameobjectWindow
   {
      nanogui::TextBox* m_objectWidth = nullptr;
      nanogui::TextBox* m_objectHeight = nullptr;
      nanogui::Button* m_textureButton = nullptr;


   };

   Editor& m_parent;
   
   const std::string GAMEOBJECT = "GAMEOBJECT WINDOW";
   const std::string TOOLS = "TOOLS WINDOW";
   
   GameobjectWindow m_currentGameObjectWindow;
   std::unordered_map< std::string, nanogui::Window* > m_windows;
};
