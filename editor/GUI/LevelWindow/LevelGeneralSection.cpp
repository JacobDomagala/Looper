#include "LevelGeneralSection.hpp"
#include "Animatable.hpp"
#include "Editor.hpp"
#include "GameObject.hpp"
#include "GuiBuilder.hpp"
#include "Utils.hpp"

namespace dgame {

LevelGeneralSection::LevelGeneralSection(Widget* parent, Editor& editor, bool activeByDefault) : Section(parent, editor, "GENERAL")
{
}

void
LevelGeneralSection::Create(std::shared_ptr< Level > level)
{
   m_loadedLevel = level;
   const auto objectSize = glm::ivec2(m_loadedLevel->GetSize());

   auto layout = GuiBuilder::CreateLayout(parent(), GuiBuilder::LayoutType::GRID);

   AddWidget(GuiBuilder::CreateLabel(layout, "Width"));

   m_width = GuiBuilder::CreateTextBox(layout, std::to_string(objectSize.x));
   AddWidget(m_width);

   AddWidget(GuiBuilder::CreateLabel(layout, "Height"));

   m_height = GuiBuilder::CreateTextBox(layout, std::to_string(objectSize.y));
   AddWidget(m_height);

   GuiBuilder::CreateLabel(parent(), "");
   AddWidget(GuiBuilder::CreateCheckBox(
      parent(), [&](bool checked) { m_editor.ShowWireframe(checked); }, "Render wireframe"));
}

void
LevelGeneralSection::Update()
{
   const auto objectSize = m_loadedLevel->GetSize();

   m_width->setValue(std::to_string(objectSize.x));
   m_height->setValue(std::to_string(objectSize.y));
}

void
LevelGeneralSection::LevelLoaded(std::shared_ptr< Level > level)
{
   m_loadedLevel = level;
   Update();
}

void
LevelGeneralSection::ObjectUpdated(int ID)
{
}

} // namespace dgame