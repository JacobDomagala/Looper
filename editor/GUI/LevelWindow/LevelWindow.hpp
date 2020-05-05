#pragma once

#include "LevelGeneralSection.hpp"
#include "LevelShaderSection.hpp"

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

 private:
   LevelGeneralSection* m_generalSection = nullptr;
   LevelShaderSection* m_shaderSection = nullptr;

   Editor& m_parent;

   bool m_created = false;

   std::shared_ptr< Level > m_loadedLevel = nullptr;
};

} // namespace dgame