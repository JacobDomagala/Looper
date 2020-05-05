#include "LevelWindow.hpp"
#include "Editor.hpp"
#include "GuiBuilder.hpp"

namespace dgame {

LevelWindow::LevelWindow(Editor& editor) : GuiWindow(&editor, "Current level"), m_parent(editor)
{
}

void
LevelWindow::Update()
{
   if (m_created)
   {
      m_generalSection->Update();
      m_shaderSection->Update();
   }
}

void
LevelWindow::LevelLoaded(std::shared_ptr< Level > loadedLevel)
{
   m_loadedLevel = loadedLevel;

   if (m_created)
   {
      m_generalSection->LevelLoaded(m_loadedLevel);
      m_shaderSection->LevelLoaded(m_loadedLevel);
   }
   else
   {
      mLayout = new GroupLayout();
      setFixedSize({300, 400});
      const auto windowSize = m_parent.GetWindowSize();
      mPos = {0, windowSize.y / 2};

      m_generalSection = new LevelGeneralSection(this, m_parent);
      m_generalSection->Create(m_loadedLevel);

      m_shaderSection = new LevelShaderSection(this, m_parent);
      m_shaderSection->Create(m_loadedLevel);

      m_created = true;
   }
}

} // namespace dgame