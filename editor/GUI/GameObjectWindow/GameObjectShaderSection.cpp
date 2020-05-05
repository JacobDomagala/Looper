#include "GameObjectShaderSection.hpp"
#include "Editor.hpp"
#include "GameObject.hpp"
#include "GuiBuilder.hpp"
#include "Utils.hpp"

namespace dgame {

GameObjectShaderSection::GameObjectShaderSection(Widget* parent, Editor& editor, bool activeByDefault)
   : Section(parent, editor, "SHADER")
{
}

void
GameObjectShaderSection::Create(std::shared_ptr< GameObject > object)
{
   m_currentlySelectedObject = object;

   m_textureChangeCallback = [&]() {
      auto filePath = nanogui::file_dialog({{"png", "Texture file"}}, false);
      if (!filePath.empty())
      {
         const auto fileName = std::filesystem::path(filePath).filename().u8string();
         m_currentlySelectedObject->GetSprite().SetTextureFromFile(fileName);
         m_textureButton->setCaption(fileName);
         m_textureView->bindImage(m_currentlySelectedObject->GetSprite().GetTexture().Create());
      }
   };

   const auto textureName = m_currentlySelectedObject->GetSprite().GetTextureName();

   auto layout = GuiBuilder::CreateLayout(parent(), GuiBuilder::LayoutType::GRID);

   AddWidget(GuiBuilder::CreateLabel(layout, "Texture"));
   m_textureButton = GuiBuilder::CreateButton(layout, textureName, m_textureChangeCallback);

   AddWidget(m_textureButton);

   m_textureView =
      GuiBuilder::CreateImageView(GuiBuilder::CreateLayout(parent(), GuiBuilder::LayoutType::GRID, Orientation::Horizontal, 1),
                                  m_currentlySelectedObject->GetSprite().GetTexture().Create(), {200, 200});
   AddWidget(m_textureView);


   AddWidget(GuiBuilder::CreateBlankSpace(layout));
}

void
GameObjectShaderSection::Update()
{
   const auto currentlySelectedTexture = m_currentlySelectedObject->GetSprite().GetTexture().GetName();

   if (currentlySelectedTexture != m_textureButton->caption())
   {
      m_textureButton->setCaption(currentlySelectedTexture);
      m_textureView->bindImage(m_currentlySelectedObject->GetSprite().GetTexture().Create());
   }
}

void
GameObjectShaderSection::ObjectUpdated(int ID)
{
}


void
GameObjectShaderSection::GameObjectSelected(std::shared_ptr< GameObject > selectedGameObject)
{
   m_currentlySelectedObject = selectedGameObject;
   Update();
}

} // namespace dgame