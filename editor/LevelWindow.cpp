#include "LevelWindow.hpp"
#include "Editor.hpp"
#include "GuiBuilder.hpp"

#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <nanovg.h>
#include <thread>

LevelWindow::LevelWindow(Editor& editor) : nanogui::Window(&editor, "Current level"), m_parent(editor)
{
}

void
LevelWindow::Update()
{
   if (m_created)
   {
      const auto objectSize = m_loadedLevel->GetSize();

      m_width->setValue(std::to_string(objectSize.x));
      m_height->setValue(std::to_string(objectSize.y));

      m_textureButton->setCaption(m_loadedLevel->GetSprite().GetTextureName());
   }
}

void
LevelWindow::LevelLoaded(Level* loadedLevel)
{
   m_loadedLevel = loadedLevel;

   auto textureButtonLambda = [&]() {
      auto filePath = nanogui::file_dialog({{"png", "Texture file"}}, false);
      if (!filePath.empty())
      {
         const auto fileName = std::filesystem::path(filePath).filename().u8string();
         m_loadedLevel->GetSprite().SetTextureFromFile(fileName);
         m_textureButton->setCaption(fileName);
      }
   };

   // Don't create new window for newly selected object
   // Just refill its content with new object's values
   if (m_created)
   {
      const auto objectSize = m_loadedLevel->GetSize();

      m_width->setValue(std::to_string(objectSize.x));
      m_height->setValue(std::to_string(objectSize.y));

      m_textureButton->setCaption(m_loadedLevel->GetSprite().GetTextureName());
      m_textureButton->setCallback(textureButtonLambda);
   }
   else
   {
      const auto windowSize = m_parent.GetWindowSize();
      mPos = nanogui::Vector2i(0, windowSize.y / 2);
      mLayout = new nanogui::GroupLayout();

      const auto objectSize = m_loadedLevel->GetSize();

      GuiBuilder::CreateLabel(this, "Width");
      m_width = GuiBuilder::CreateTextBox(this, std::to_string(objectSize.x));

      GuiBuilder::CreateLabel(this, "Height");
      m_height = GuiBuilder::CreateTextBox(this, std::to_string(objectSize.y));

      const auto textureName = m_loadedLevel->GetSprite().GetTextureName();
      GuiBuilder::CreateLabel(this, "Texture");
      m_textureButton = GuiBuilder::CreateButton(this, textureName, textureButtonLambda);

      GuiBuilder::CreateLabel(this, "");
      GuiBuilder::CreateCheckBox(
         this, [&](bool checked) { m_parent.ShowWireframe(checked); }, "Render wireframe");

      m_created = true;
   }
}