#include "GameObjectGeneralSection.hpp"
#include "Animatable.hpp"
#include "Editor.hpp"
#include "GameObject.hpp"
#include "GuiBuilder.hpp"
#include "Utils.hpp"

#include <nanogui/entypo.h>

namespace dgame {

GameObjectGeneralSection::GameObjectGeneralSection(Widget* parent, Editor& editor, bool activeByDefault)
   : Section(parent, editor, "GENERAL")
{
}

void
GameObjectGeneralSection::Create(std::shared_ptr< GameObject > selectedGameObject)
{
   m_currentlySelectedObject = selectedGameObject;

   const auto numColumns = 3;
   const auto fixedWidth = parent()->fixedSize().x() / numColumns;
   const auto objectMapPosition = m_currentlySelectedObject->GetCenteredLocalPosition();
   const auto objectSize = m_currentlySelectedObject->GetSize();

   auto nameLayout =
      GuiBuilder::CreateLayout(parent(), GuiBuilder::LayoutType::GRID, Orientation::Horizontal, 2, Alignment::Fill, 2);

   AddWidget(GuiBuilder::CreateLabel(nameLayout, "Name"));

   m_name = GuiBuilder::CreateTextBox(
      nameLayout, m_currentlySelectedObject->GetName(),
      [&](const std::string& newName) {
         m_currentlySelectedObject->SetName(newName);
         return true;
      },
      false, {parent()->fixedSize().x() / 2, 25}, true);
   AddWidget(m_name);

   AddWidget(GuiBuilder::CreateLabel(nameLayout, "Type"));

   m_type = GuiBuilder::CreateTextBox(
      nameLayout, m_currentlySelectedObject->GetTypeString(), [](const std::string& newName) { return true; }, false,
      {parent()->fixedSize().x() / 2, 25}, false);
   AddWidget(m_type);

   auto layout =
      GuiBuilder::CreateLayout(parent(), GuiBuilder::LayoutType::GRID, Orientation::Horizontal, 3, Alignment::Fill);

   AddWidget(GuiBuilder::CreateLabel(layout, "Size"));

   m_width = GuiBuilder::CreateTextBox(layout, std::to_string(objectSize.x));
   m_width->setFixedWidth(fixedWidth);
   AddWidget(m_width);

   m_height = GuiBuilder::CreateTextBox(layout, std::to_string(objectSize.y));
   m_height->setFixedWidth(fixedWidth);
   AddWidget(m_height);

   AddWidget(GuiBuilder::CreateLabel(layout, "Position"));

   m_mapPositionX = GuiBuilder::CreateTextBox(layout, std::to_string(objectMapPosition.x));
   m_mapPositionX->setFixedWidth(fixedWidth);
   AddWidget(m_mapPositionX);

   m_mapPositionY = GuiBuilder::CreateTextBox(layout, std::to_string(objectMapPosition.y));
   m_mapPositionY->setFixedWidth(fixedWidth);
   AddWidget(m_mapPositionY);

   AddWidget(GuiBuilder::CreateBlankSpace(layout));
}

void
GameObjectGeneralSection::Update()
{
   const auto objectMapPosition = m_currentlySelectedObject->GetCenteredLocalPosition();
   const auto objectSize = m_currentlySelectedObject->GetSize();

   m_width->setValue(std::to_string(objectSize.x));
   m_height->setValue(std::to_string(objectSize.y));
   m_mapPositionX->setValue(std::to_string(objectMapPosition.x));
   m_mapPositionY->setValue(std::to_string(objectMapPosition.y));
}

void
GameObjectGeneralSection::GameObjectSelected(std::shared_ptr< GameObject > selectedGameObject)
{
   m_currentlySelectedObject = selectedGameObject;
   m_nameChangeCallback = [&]() { m_currentlySelectedObject->SetName(m_name->value()); };

   const auto objectSize = m_currentlySelectedObject->GetSize();
   const auto objectMapPosition = glm::ivec2(m_currentlySelectedObject->GetCenteredLocalPosition());

   m_width->setValue(std::to_string(objectSize.x));
   m_height->setValue(std::to_string(objectSize.y));
   m_mapPositionX->setValue(std::to_string(objectMapPosition.x));
   m_mapPositionY->setValue(std::to_string(objectMapPosition.y));
   m_name->setValue(m_currentlySelectedObject->GetName());
   m_type->setValue(m_currentlySelectedObject->GetTypeString());
}

void
GameObjectGeneralSection::ObjectUpdated(dgame::Object::ID ID)
{
}

void
GameObjectGeneralSection::ObjectDeleted(dgame::Object::ID ID)
{
}

} // namespace dgame