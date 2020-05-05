#pragma once

#include "Object.hpp"
#include "Section.hpp"

#include <string>
#include <vector>

namespace dgame {

class Level;
class Editor;

class LevelGeneralSection : public Section
{
 public:
   struct NodePoint
   {
      int id;
      nanogui::TextBox* m_xPos = nullptr;
      nanogui::TextBox* m_yPos = nullptr;
      nanogui::Button* m_removePoint = nullptr;
   };

 public:
   LevelGeneralSection(nanogui::Widget* parent, Editor& editor, bool activeByDefault = true);

   void
   Update();

   void
   Create(std::shared_ptr< Level > selectedGameObject);

   void
   ObjectUpdated(int ID);

   void
   LevelLoaded(std::shared_ptr< Level > loadedLevel);

 private:
   nanogui::TextBox* m_width = nullptr;
   nanogui::TextBox* m_height = nullptr;
   nanogui::CheckBox* m_wireframe = nullptr;
   nanogui::CheckBox* m_showCollision = nullptr;
   nanogui::Widget* m_pathfinderNodesLayout = nullptr;
   std::pair< nanogui::PopupButton*, nanogui::Popup* > m_showNodesButton;
   std::vector< NodePoint > m_pathfinderNodes;

   std::shared_ptr< Level > m_loadedLevel;
   dgame::Object::VectorPtr m_objects;
};

} // namespace dgame