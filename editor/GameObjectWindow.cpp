#include "GameObjectWindow.hpp"
#include "GuiBuilder.hpp"
#include "Editor.hpp"
#include "GameObject.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <nanovg.h>
#include <fstream>
#include <thread>

GameObjectWindow::GameObjectWindow(Editor& editor) : nanogui::Window(&editor, "Selected Object"), m_parent(editor)
{

}

void
GameObjectWindow::Update()
{
   if (mVisible)
   {
      const auto windowSize = m_parent.GetWindowSize();
      const auto objectSize = m_currentlySelectedObject->GetSize();

      m_objectWidth->setValue(std::to_string(objectSize.x));
      m_objectHeight->setValue(std::to_string(objectSize.y));

      m_textureButton->setCaption(m_currentlySelectedObject->GetSprite().GetTextureName());
      mPos = nanogui::Vector2i(windowSize.x - mSize.x(), 0);
   }
}

void
GameObjectWindow::GameObjectSelected(std::shared_ptr< GameObject > selectedGameObject)
{
   m_currentlySelectedObject = selectedGameObject;
   const auto windowSize = m_parent.GetWindowSize();

   auto textureButtonLambda = [&]() {
      auto filePath = nanogui::file_dialog({{"png", "Texture file"}}, false);
      if (!filePath.empty())
      {
         const auto fileName = std::filesystem::path(filePath).filename().u8string();
         m_currentlySelectedObject->GetSprite().SetTextureFromFile(fileName);
         m_textureButton->setCaption(fileName);
      }
   };

   // Don't create new window for newly selected object
   // Just refill its content with new object's values
   if (m_created)
   {
      const auto objectSize = m_currentlySelectedObject->GetSize();

      m_objectWidth->setValue(std::to_string(objectSize.x));
      m_objectHeight->setValue(std::to_string(objectSize.y));

      m_textureButton->setCaption(m_currentlySelectedObject->GetSprite().GetTextureName());
      m_textureButton->setCallback(textureButtonLambda);
   }
   else
   {
      mLayout = new nanogui::GroupLayout();
    
      const auto objectSize = m_currentlySelectedObject->GetSize();

      GuiBuilder::CreateLabel(this, "Width");
      m_objectWidth = GuiBuilder::CreateTextBox(this, std::to_string(objectSize.x));

      GuiBuilder::CreateLabel(this, "Height");
      m_objectHeight = GuiBuilder::CreateTextBox(this, std::to_string(objectSize.y));

      const auto textureName = m_currentlySelectedObject->GetSprite().GetTextureName();
      GuiBuilder::CreateLabel(this, "Texture");
      m_textureButton = GuiBuilder::CreateButton(this, textureName, textureButtonLambda);

      m_created = true;
   }

   mVisible = true;
}

void
GameObjectWindow::GameObjectUnselected()
{
   m_currentlySelectedObject.reset();
   mVisible = false;
}