#pragma once

#include "GuiAPI.hpp"
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
   LevelGeneralSection(Widget* parent, Editor& editor, bool activeByDefault = true);

   void
   Update();

   void
   Create(std::shared_ptr< Level > selectedGameObject);

   void
   ObjectUpdated(int ID);

   void
   LevelLoaded(std::shared_ptr< Level > loadedLevel);

 private:
   TextBox* m_width = nullptr;
   TextBox* m_height = nullptr;
   CheckBox* m_wireframe = nullptr;
   std::shared_ptr< Level > m_loadedLevel;
   dgame::Object::VectorPtr m_objects;
};

} // namespace dgame