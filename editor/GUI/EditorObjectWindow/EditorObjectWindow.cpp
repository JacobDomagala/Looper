#include "EditorObjectWindow.hpp"
#include "Editor.hpp"
#include "Enemy.hpp"
#include "GameObject.hpp"
#include "GuiBuilder.hpp"
#include "Utils.hpp"

EditorObjectWindow::EditorObjectWindow(Editor& editor) : nanogui::Window(&editor, "Editor Object"), m_parent(editor)
{
}

void
EditorObjectWindow::ObjectUpdated(int ID)
{
}

void
EditorObjectWindow::Update()
{
}

void
EditorObjectWindow::EditorObjectSelected(std::shared_ptr< EditorObject > selectedEditorObject)
{
   m_currentlySelectedObject = selectedEditorObject;
   
   if(m_created)
   {
      m_type->setValue(m_currentlySelectedObject->GetLinkedObject()->GetTypeString());
   }
   else
   {
      mLayout = new nanogui::GroupLayout();
      auto windowsize = m_parent.GetWindowSize();
      mFixedSize = {300, 300};

      mPos = {windowsize.x - (2 * mFixedSize.x()), windowsize.y - mFixedSize.y()};

      auto nameLayout =
         GuiBuilder::CreateLayout(this, GuiBuilder::LayoutType::GRID, nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Fill, 2);

      GuiBuilder::CreateLabel(nameLayout, "Type");

      m_type = GuiBuilder::CreateTextBox(
         nameLayout, m_currentlySelectedObject->GetLinkedObject()->GetTypeString(), [](const std::string& newName) { return true; }, false,
         {fixedSize().x() / 2, 25}, false);
   }
   m_created = true;
   mVisible = true;
}

void
EditorObjectWindow::EditorObjectUnselected()
{
   m_currentlySelectedObject.reset();
   m_created = false;
   mVisible = false;
}
