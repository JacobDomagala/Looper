#pragma once

#include "LevelGeneralSection.hpp"
#include "LevelPathfinderSection.hpp"
#include "LevelShaderSection.hpp"
#include "Object.hpp"

#include <functional>
#include <nanogui/window.h>

namespace dgame {

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
   LevelLoaded(std::shared_ptr< Level > loadedLevel);

   void
   ObjectUpdated(dgame::Object::ID ID);

   void
   ObjectDeleted(dgame::Object::ID ID);

 private:
   LevelGeneralSection* m_generalSection = nullptr;
   LevelPathfinderSection* m_pathfinderSection = nullptr;
   LevelShaderSection* m_shaderSection = nullptr;

   Editor& m_parent;

   bool m_created = false;

   std::shared_ptr< Level > m_loadedLevel = nullptr;
   dgame::Object::VectorPtr m_objects;
};

} // namespace dgame