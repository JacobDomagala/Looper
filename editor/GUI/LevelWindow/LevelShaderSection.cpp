#include "LevelShaderSection.hpp"
#include "Editor.hpp"
#include "GameObject.hpp"
#include "GuiBuilder.hpp"
#include "Utils.hpp"

#include <nanogui/imageview.h>

LevelShaderSection::LevelShaderSection(nanogui::Widget* parent, Editor& editor, bool activeByDefault) : Section(parent, editor, "SHADER")
{
}

void
LevelShaderSection::Create(std::shared_ptr< Level > level)
{
   m_loadedLevel = level;

   auto layout = GuiBuilder::CreateLayout(parent(), GuiBuilder::LayoutType::GRID);

   const auto textureName = m_loadedLevel->GetSprite().GetTextureName();
   AddWidget(GuiBuilder::CreateLabel(layout, "Texture"));
   m_textureButton = GuiBuilder::CreateButton(layout, textureName, m_textureChangeCallback);

   AddWidget(m_textureButton);
}

void
LevelShaderSection::Update()
{
   auto textureButtonLambda = [&]() {
      auto filePath = nanogui::file_dialog({{"png", "Texture file"}}, false);
      if (!filePath.empty())
      {
         const auto fileName = std::filesystem::path(filePath).filename().u8string();
         m_loadedLevel->GetSprite().SetTextureFromFile(fileName);
         m_textureButton->setCaption(fileName);
      }
   };

   m_textureButton->setCaption(m_loadedLevel->GetSprite().GetTextureName());
   m_textureButton->setCallback(textureButtonLambda);
}

void
LevelShaderSection::LevelLoaded(std::shared_ptr< Level > level)
{
   m_loadedLevel = level;
   Update();
}

void
LevelShaderSection::ObjectUpdated(int ID)
{
}
