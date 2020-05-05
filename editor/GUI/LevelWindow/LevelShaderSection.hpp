#pragma once

#include "Object.hpp"
#include "Section.hpp"

#include <string>
#include <vector>

namespace dgame {

class Level;
class Editor;

class LevelShaderSection : public Section
{
 public:
   LevelShaderSection(nanogui::Widget* parent, Editor& editor, bool activeByDefault = true);

   void
   Update();

   void
   Create(std::shared_ptr< Level > object);

   void
   ObjectUpdated(int ID);

   void
   LevelLoaded(std::shared_ptr< Level > loadedLevel);

 private:
   nanogui::Button* m_textureButton = nullptr;
   nanogui::Button* m_shaderButton = nullptr;

   std::shared_ptr< Level > m_loadedLevel;
   dgame::Object::VectorPtr m_objects;

   std::function< void() > m_textureChangeCallback;
};

} // namespace dgame