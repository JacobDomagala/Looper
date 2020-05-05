#pragma once

#include "Object.hpp"

#include <functional>
#include <nanogui/window.h>

namespace dgame {

class Editor;
class EditorObject;

class EditorObjectWindow : public nanogui::Window
{
 public:
   EditorObjectWindow(Editor& parent);
   ~EditorObjectWindow() = default;

   void
   Update();

   void
   EditorObjectSelected(std::shared_ptr< EditorObject > selectedEditorObject);

   void
   EditorObjectUnselected();

   void
   ObjectUpdated(int ID);

 private:
   Editor& m_parent;

   nanogui::TextBox* m_type = nullptr;
   nanogui::TextBox* m_xPos = nullptr;
   nanogui::TextBox* m_yPos = nullptr;

   bool m_created = false;
   dgame::Object::VectorPtr m_objects;
   std::shared_ptr< EditorObject > m_currentlySelectedObject;
};

} // namespace dgame