#pragma once

#include "Object.hpp"
#include "Section.hpp"

#include <string>
#include <vector>

namespace dgame {

class GameObject;
class Editor;

class GameObjectGeneralSection : public Section
{
 public:
   GameObjectGeneralSection(nanogui::Widget* parent, Editor& editor, bool activeByDefault = true);

   void
   Update();

   void
   Create(std::shared_ptr< GameObject > selectedGameObject);

   void
   ObjectUpdated(dgame::Object::ID ID);

   void
   ObjectDeleted(dgame::Object::ID ID);

   void
   GameObjectSelected(std::shared_ptr< GameObject > selectedGameObject);

 private:
   nanogui::TextBox* m_width = nullptr;
   nanogui::TextBox* m_height = nullptr;
   nanogui::TextBox* m_mapPositionX = nullptr;
   nanogui::TextBox* m_mapPositionY = nullptr;
   nanogui::TextBox* m_name = nullptr;
   nanogui::TextBox* m_type = nullptr;

   std::shared_ptr< GameObject > m_currentlySelectedObject;

   std::function< void() > m_nameChangeCallback;
};

} // namespace dgame