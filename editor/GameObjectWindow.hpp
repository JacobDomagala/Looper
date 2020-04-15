#pragma once

#include <nanogui/window.h>

class Editor;
class GameObject;

class GameObjectWindow : public nanogui::Window
{
 public:
   GameObjectWindow(Editor& parent);
   ~GameObjectWindow() = default;

   void
   Update();

   void
   GameObjectSelected(std::shared_ptr< GameObject > selectedGameObject);

   void
   GameObjectUnselected();

 private:
   nanogui::TextBox* m_objectWidth = nullptr;
   nanogui::TextBox* m_objectHeight = nullptr;
   nanogui::TextBox* m_objectGlobalPosition = nullptr;
   nanogui::TextBox* m_objectMapPosition = nullptr;
   nanogui::Button* m_textureButton = nullptr;
   nanogui::Button* m_shaderButton = nullptr;
   nanogui::CheckBox* m_animate = nullptr;
   nanogui::CheckBox* m_showCollision = nullptr;

   Editor& m_parent;

   bool m_created = false;
   std::shared_ptr< GameObject > m_currentlySelectedObject;
};
