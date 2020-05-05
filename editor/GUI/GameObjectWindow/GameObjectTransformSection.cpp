#include "GameObjectTransformSection.hpp"
#include "Animatable.hpp"
#include "Editor.hpp"
#include "GameObject.hpp"
#include "GuiBuilder.hpp"
#include "Utils.hpp"

namespace dgame {

GameObjectTransformSection::GameObjectTransformSection(Widget* parent, Editor& editor, bool activeByDefault)
   : Section(parent, editor, "TRANSFORM")
{
}

void
GameObjectTransformSection::Create(std::shared_ptr< GameObject > selectedGameObject)
{
   m_currentlySelectedObject = selectedGameObject;

   // ROTATE
   auto rotateLayout = GuiBuilder::CreateLayout(parent(), GuiBuilder::LayoutType::GRID, Orientation::Horizontal, 3);

   const auto rotationRange = std::make_pair(-360.0f, 360.0f);
   AddWidget(GuiBuilder::CreateLabel(rotateLayout, "Rotate"));

   m_rotateValue = GuiBuilder::CreateNumericBox< int32_t >(rotateLayout, m_currentlySelectedObject->GetSprite().GetRotation(),
                                                           rotationRange, [&](const int32_t& val) {
                                                              m_currentlySelectedObject->Rotate(glm::radians(static_cast< float >(val)));
                                                              return true;
                                                           });
   m_rotateValue->setFixedWidth(parent()->fixedSize().x() / 4);
   AddWidget(m_rotateValue);

   m_rotateSlider = GuiBuilder::CreateSlider(
      rotateLayout, [&](float val) { m_currentlySelectedObject->Rotate(val); },
      {glm::radians(rotationRange.first), glm::radians(rotationRange.second)}, m_currentlySelectedObject->GetSprite().GetRotation());

   m_rotateSlider->setFixedWidth(parent()->fixedSize().x() / 2);
   AddWidget(m_rotateSlider);
   AddWidget(GuiBuilder::CreateBlankSpace(rotateLayout));

   // SCALE
   const auto scaleRange = std::make_pair(1, 3);
   const auto scaleValue =
      m_currentlySelectedObject->GetSprite().GetScale() + m_currentlySelectedObject->GetSprite().GetUniformScaleValue();

   auto scaleLayout = GuiBuilder::CreateLayout(parent(), GuiBuilder::LayoutType::GRID);

   AddWidget(GuiBuilder::CreateLabel(scaleLayout, "Scale"));

   m_scaleUniformSlider = GuiBuilder::CreateSlider(
      scaleLayout,
      [&](float val) {
         m_currentlySelectedObject->GetSprite().ScaleUniformly(val);

         const auto scaleValue =
            m_currentlySelectedObject->GetSprite().GetScale() + m_currentlySelectedObject->GetSprite().GetUniformScaleValue();
      },
      {0, 2}, m_currentlySelectedObject->GetSprite().GetUniformScaleValue());

   m_scaleUniformSlider->setFixedWidth(2 * parent()->fixedSize().x() / 3);
   AddWidget(m_scaleUniformSlider);

   auto scaleLayout2 =
      GuiBuilder::CreateLayout(parent(), GuiBuilder::LayoutType::GRID, Orientation::Horizontal, 3, Alignment::Middle);

   AddWidget(GuiBuilder::CreateLabel(scaleLayout2, "X"));
   m_scaleXValue = GuiBuilder::CreateFloatingPointBox(scaleLayout2, scaleValue.x, scaleRange, [&](const std::string& val) {
      const auto currentScaleValY = m_currentlySelectedObject->GetSprite().GetScale().y;
      m_currentlySelectedObject->Scale({std::stof(val), currentScaleValY});
      return true;
   });

   m_scaleXValue->setFixedWidth(parent()->fixedSize().x() / 5);
   AddWidget(m_scaleXValue);

   m_scaleXSlider = GuiBuilder::CreateSlider(
      scaleLayout2,
      [&](float val) {
         const auto currentScaleValY = m_currentlySelectedObject->GetSprite().GetScale().y;
         m_currentlySelectedObject->Scale({val, currentScaleValY});
         return true;
      },
      scaleRange, scaleValue.x);

   m_scaleXSlider->setFixedWidth(parent()->fixedSize().x() / 3);
   AddWidget(m_scaleXSlider);

   AddWidget(GuiBuilder::CreateLabel(scaleLayout2, "Y"));
   m_scaleYValue = GuiBuilder::CreateFloatingPointBox(scaleLayout2, scaleValue.y, scaleRange, [&](const std::string& val) {
      const auto currentScaleValX = m_currentlySelectedObject->GetSprite().GetScale().x;
      m_currentlySelectedObject->Scale({currentScaleValX, std::stof(val)});
      return true;
   });

   m_scaleYValue->setFixedWidth(parent()->fixedSize().x() / 5);
   AddWidget(m_scaleYValue);

   m_scaleYSlider = GuiBuilder::CreateSlider(
      scaleLayout2,
      [&](float val) {
         const auto currentScaleValX = m_currentlySelectedObject->GetSprite().GetScale().x;
         m_currentlySelectedObject->Scale({currentScaleValX, val});
         return true;
      },
      scaleRange, scaleValue.y);

   m_scaleYSlider->setFixedWidth(parent()->fixedSize().x() / 3);
   AddWidget(m_scaleYSlider);

   AddWidget(GuiBuilder::CreateBlankSpace(scaleLayout2));
}

void
GameObjectTransformSection::Update()
{
   const auto rotationValue = m_currentlySelectedObject->GetSprite().GetRotation(Sprite::RotationType::DEGREES);
   const auto scaleValue =
      m_currentlySelectedObject->GetSprite().GetScale() + m_currentlySelectedObject->GetSprite().GetUniformScaleValue();

   m_rotateValue->setValue(rotationValue);
   m_rotateSlider->setValue(glm::radians(rotationValue));
   m_scaleUniformSlider->setValue(m_currentlySelectedObject->GetSprite().GetUniformScaleValue());
   m_scaleXValue->setValue(CustomFloatToStr(scaleValue.x));
   m_scaleXSlider->setValue(scaleValue.x);
   m_scaleYValue->setValue(CustomFloatToStr(scaleValue.y));
   m_scaleYSlider->setValue(scaleValue.y);
}

void
GameObjectTransformSection::GameObjectSelected(std::shared_ptr< GameObject > selectedGameObject)
{
   m_currentlySelectedObject = selectedGameObject;
   m_rotateSlider->setValue(m_currentlySelectedObject->GetSprite().GetRotation(Sprite::RotationType::DEGREES));
   m_scaleUniformSlider->setValue(m_currentlySelectedObject->GetSprite().GetScale().x);
}

void
GameObjectTransformSection::ObjectUpdated(int ID)
{
}

} // namespace dgame