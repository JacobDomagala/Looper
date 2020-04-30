#pragma once

#include "Object.hpp"
#include "Section.hpp"

#include <string>
#include <vector>

class Level;
class Editor;

class LevelGeneralSection : public Section
{
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

   std::shared_ptr< Level > m_loadedLevel;
   ::Object::VectorPtr m_objects;
};
