#pragma once

#include "GuiAPI.hpp"
#include "Object.hpp"
#include "Section.hpp"

#include <string>
#include <vector>

namespace dgame {

class Level;
class Editor;

class LevelPathfinderSection : public Section
{
 public:
   struct NodePoint
   {
      int id;
      TextBox* m_xPos = nullptr;
      TextBox* m_yPos = nullptr;
      Button* m_removePoint = nullptr;
   };

 public:
   LevelPathfinderSection(Widget* parent, Editor& editor, bool activeByDefault = true);

   void
   Update();

   void
   Create(std::shared_ptr< Level > selectedGameObject);

   void
   ObjectUpdated(int ID);

   void
   LevelLoaded(std::shared_ptr< Level > loadedLevel);

 private:
   CheckBox* m_showWayPoints = nullptr;
   Widget* m_pathfinderNodesLayout = nullptr;
   std::pair< PopupBtn*, Popup* > m_showNodesButton;
   std::vector< NodePoint > m_pathfinderNodes;

   std::shared_ptr< Level > m_loadedLevel;
   dgame::Object::VectorPtr m_objects;
};

} // namespace dgame