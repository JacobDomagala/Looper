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
      setFixedSize(nanogui::Vector2i(250, 400));

      CreateGeneralSection();
      CreateShaderSection();

      GuiBuilder::CreateLabel(this, "");
      GuiBuilder::CreateCheckBox(
         this, [&](bool checked) { m_parent.ShowWireframe(checked); }, "Render wireframe");

      m_created = true;
   }
}

void
LevelWindow::CreateGeneralSection()
{
   const auto objectSize = glm::ivec2(m_loadedLevel->GetSize());

   m_generalSection = GuiBuilder::CreateSection(this, "General");

   auto layout = GuiBuilder::CreateLayout(this, GuiBuilder::LayoutType::GRID);

   m_generalSection->AddWidget(GuiBuilder::CreateLabel(layout, "Width"));

   m_width = GuiBuilder::CreateTextBox(layout, std::to_string(objectSize.x));
   m_generalSection->AddWidget(m_width);

   m_generalSection->AddWidget(GuiBuilder::CreateLabel(layout, "Height"));

   m_height = GuiBuilder::CreateTextBox(layout, std::to_string(objectSize.y));
   m_generalSection->AddWidget(m_height);
}

void
LevelWindow::CreateShaderSection()
{
   m_shaderSection = GuiBuilder::CreateSection(this, "Shader");

   auto layout = GuiBuilder::CreateLayout(this, GuiBuilder::LayoutType::GRID);

   const auto textureName = m_loadedLevel->GetSprite().GetTextureName();
   m_shaderSection->AddWidget(GuiBuilder::CreateLabel(layout, "Texture"));
   m_textureButton = GuiBuilder::CreateButton(layout, textureName, m_textureChangeCallback);

   m_shaderSection->AddWidget(m_textureButton);
}