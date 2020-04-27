#include "GameObjectWindow.hpp"
#include "Editor.hpp"
#include "Enemy.hpp"
#include "GameObject.hpp"
#include "GuiBuilder.hpp"

#include "Utils.hpp"

#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <nanogui/imageview.h>

#include <nanovg.h>
#include <thread>

GameObjectWindow::GameObjectWindow(Editor& editor) : nanogui::Window(&editor, "Selected Object"), m_parent(editor)
{
}

void
GameObjectWindow::ObjectUpdated(int ID)
{
   auto object = std::find_if(m_objects.begin(), m_objects.end(), [ID](auto& object) { return object->GetID() == ID; });

   if (object != m_objects.end())
   {
      switch ((*object)->GetType())
      {
         case ::Object::TYPE::PLAYER:
         case ::Object::TYPE::ENEMY: {
            UpdateGeneralSection();
            UpdateTransformSection();
            UpdateShaderSection();
         }
         break;

         case ::Object::TYPE::ANIMATION_POINT: {
            auto point = std::find_if(m_animationSteps.begin(), m_animationSteps.end(), [ID](auto& point) { return point.id == ID; });

            if (point != m_animationSteps.end())
            {
               auto animatioinPoint = std::dynamic_pointer_cast<::AnimationPoint >(*object);

               point->m_xPos->setValue(CustomFloatToStr(animatioinPoint->m_end.x));
               point->m_yPos->setValue(CustomFloatToStr(animatioinPoint->m_end.y));
               point->m_rotation->setValue(CustomFloatToStr(0.0f));
               point->m_time->setValue(CustomFloatToStr(animatioinPoint->m_timeDuration.count()));
            }
         }
         break;

         default: {
         }
      }
   }
}

void
GameObjectWindow::Update()
{
   UpdateAnimationSection();
}

void
GameObjectWindow::GameObjectSelected(std::shared_ptr< GameObject > selectedGameObject)
{
   m_currentlySelectedObject = selectedGameObject;

   if (m_objects.end() == std::find(m_objects.begin(), m_objects.end(), std::dynamic_pointer_cast<::Object >(selectedGameObject)))
   {
      m_objects.push_back(std::dynamic_pointer_cast<::Object >(selectedGameObject));
   }

   const auto windowSize = m_parent.GetWindowSize();
   const auto frameBuffersize = m_parent.GetFrameBufferwSize();

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

   m_nameChangeCallback = [&]() { m_currentlySelectedObject->SetName(m_name->value()); };

   // Don't create new window for newly selected object
   // Just refill its content with new object's values
   if (m_created)
   {
      const auto objectSize = m_currentlySelectedObject->GetSize();
      const auto objectMapPosition = glm::ivec2(m_currentlySelectedObject->GetCenteredLocalPosition());

      m_textureButton->setCallback(m_textureChangeCallback);

      m_width->setValue(std::to_string(objectSize.x));
      m_height->setValue(std::to_string(objectSize.y));
      m_mapPositionX->setValue(std::to_string(objectMapPosition.x));
      m_mapPositionY->setValue(std::to_string(objectMapPosition.y));
      m_name->setValue(m_currentlySelectedObject->GetName());
      m_type->setValue(m_currentlySelectedObject->GetTypeString());

      m_rotateSlider->setValue(m_currentlySelectedObject->GetSprite().GetRotation(Sprite::RotationType::DEGREES));
      m_scaleUniformSlider->setValue(m_currentlySelectedObject->GetSprite().GetScale().x);

      m_animationSection->setActive(m_currentlySelectedObject->GetType() != GameObject::TYPE::PLAYER);

      const auto animatablePtr = std::dynamic_pointer_cast< Animatable >(m_currentlySelectedObject);

      if (animatablePtr)
      {
         const auto type = animatablePtr->GetAnimationType();

         m_loopAnimationButton->setPushed(type == Animatable::ANIMATION_TYPE::LOOP);
         m_reversalAnimationButton->setPushed(type == Animatable::ANIMATION_TYPE::REVERSABLE);
         m_showAnimationSteps->setChecked(animatablePtr->GetRenderAnimationSteps());
         m_lockAnimationSteps->setChecked(animatablePtr->GetLockAnimationSteps());
         m_animationTimeSlider->setRange({0, animatablePtr->GetAnimationDuration().count()});
         m_animationTimeSlider->setValue(0);
      }

      ClearAnimationSteps();
      CreateAnimationSteps();
   }
   else
   {
      mLayout = new nanogui::GroupLayout();

      /* m_selectedTheme = new nanogui::Theme(m_parent.nvgContext());
       m_unSelectedTheme = new nanogui::Theme(m_parent.nvgContext());

       m_selectedTheme->mTextColor = {255, 0, 0, 255};
       m_unSelectedTheme->mTextColor = {255, 255, 255, 255};*/

      setFixedSize(nanogui::Vector2i(300, frameBuffersize.y));

      CreateGeneralSection();
      CreateTransformSection();
      CreateShaderSection();
      CreateAnimationSection();

      m_created = true;
   }

   mPos = nanogui::Vector2i(frameBuffersize.x - 300, 0);
   mVisible = true;
}

void
GameObjectWindow::GameObjectUnselected()
{
   m_currentlySelectedObject.reset();
   mVisible = false;
}

void
GameObjectWindow::AnimationPointSelected(int ID)
{
   /*if (m_currentlySelectedObject)
   {
      const auto animatablePtr = std::dynamic_pointer_cast< Animatable >(m_currentlySelectedObject);

      if (animatablePtr)
      {
         auto it = std::find_if(m_animationSteps.begin(), m_animationSteps.end(), [ID](auto& point) { return point.id == ID; });

         if (it != m_animationSteps.end())
         {
            it->m_xPos->setTheme(m_selectedTheme);
            it->m_yPos->setTheme(m_selectedTheme);
            it->m_rotation->setTheme(m_selectedTheme);
            it->m_time->setTheme(m_selectedTheme);
         }
      }
   }*/
}

void
GameObjectWindow::AnimationPointUnselected(int ID)
{
   // if (m_currentlySelectedObject)
   //{
   //   const auto animatablePtr = std::dynamic_pointer_cast< Animatable >(m_currentlySelectedObject);

   //   if (animatablePtr)
   //   {
   //      auto it = std::find_if(m_animationSteps.begin(), m_animationSteps.end(), [ID](auto& point) { return point.id == ID; });

   //      if (it != m_animationSteps.end())
   //      {
   //         it->m_xPos->setTheme(m_unSelectedTheme);
   //         it->m_yPos->setTheme(m_unSelectedTheme);
   //         it->m_rotation->setTheme(m_unSelectedTheme);
   //         it->m_time->setTheme(m_unSelectedTheme);
   //      }
   //   }
   //}
}

void
GameObjectWindow::CreateGeneralSection()
{
   const auto numColumns = 3;
   const auto fixedWidth = mFixedSize.x() / numColumns;
   const auto objectMapPosition = m_currentlySelectedObject->GetCenteredLocalPosition();
   const auto objectSize = m_currentlySelectedObject->GetSize();

   m_generalSection = GuiBuilder::CreateSection(this, "General");

   auto nameLayout =
      GuiBuilder::CreateLayout(this, GuiBuilder::LayoutType::GRID, nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Fill);

   m_generalSection->AddWidget(GuiBuilder::CreateLabel(nameLayout, "Name"));

   m_name = GuiBuilder::CreateTextBox(
      nameLayout, m_currentlySelectedObject->GetName(),
      [&](const std::string& newName) {
         m_currentlySelectedObject->SetName(newName);
         return true;
      },
      false, {mFixedSize.x() / 2, 25}, true);
   m_generalSection->AddWidget(m_name);

   m_generalSection->AddWidget(GuiBuilder::CreateLabel(nameLayout, "Type"));

   m_type = GuiBuilder::CreateTextBox(
      nameLayout, m_currentlySelectedObject->GetTypeString(), [](const std::string& newName) { return true; }, false,
      {mFixedSize.x() / 2, 25}, false);
   m_generalSection->AddWidget(m_type);

   auto layout =
      GuiBuilder::CreateLayout(this, GuiBuilder::LayoutType::GRID, nanogui::Orientation::Horizontal, 3, nanogui::Alignment::Fill);

   m_generalSection->AddWidget(GuiBuilder::CreateLabel(layout, "Size"));

   m_width = GuiBuilder::CreateTextBox(layout, std::to_string(objectSize.x));
   m_width->setFixedWidth(fixedWidth);
   m_generalSection->AddWidget(m_width);

   m_height = GuiBuilder::CreateTextBox(layout, std::to_string(objectSize.y));
   m_height->setFixedWidth(fixedWidth);
   m_generalSection->AddWidget(m_height);

   m_generalSection->AddWidget(GuiBuilder::CreateLabel(layout, "Position"));

   m_mapPositionX = GuiBuilder::CreateTextBox(layout, std::to_string(objectMapPosition.x));
   m_mapPositionX->setFixedWidth(fixedWidth);
   m_generalSection->AddWidget(m_mapPositionX);

   m_mapPositionY = GuiBuilder::CreateTextBox(layout, std::to_string(objectMapPosition.y));
   m_mapPositionY->setFixedWidth(fixedWidth);
   m_generalSection->AddWidget(m_mapPositionY);

   m_generalSection->AddWidget(GuiBuilder::CreateBlankSpace(layout));
}

void
GameObjectWindow::UpdateGeneralSection()
{
   const auto objectMapPosition = m_currentlySelectedObject->GetCenteredLocalPosition();
   const auto objectSize = m_currentlySelectedObject->GetSize();

   m_width->setValue(std::to_string(objectSize.x));
   m_height->setValue(std::to_string(objectSize.y));
   m_mapPositionX->setValue(std::to_string(objectMapPosition.x));
   m_mapPositionY->setValue(std::to_string(objectMapPosition.y));
}

void
GameObjectWindow::CreateTransformSection()
{
   m_transformSection = GuiBuilder::CreateSection(this, "Transform");

   // ROTATE
   auto rotateLayout = GuiBuilder::CreateLayout(this, GuiBuilder::LayoutType::GRID, nanogui::Orientation::Horizontal, 3);

   const auto rotationRange = std::make_pair(-360.0f, 360.0f);
   m_transformSection->AddWidget(GuiBuilder::CreateLabel(rotateLayout, "Rotate"));

   m_rotateValue = GuiBuilder::CreateNumericBox< int32_t >(rotateLayout, m_currentlySelectedObject->GetSprite().GetRotation(),
                                                           rotationRange, [&](const int32_t& val) {
                                                              m_currentlySelectedObject->Rotate(glm::radians(static_cast< float >(val)));
                                                              return true;
                                                           });
   m_rotateValue->setFixedWidth(mFixedSize.x() / 4);
   m_transformSection->AddWidget(m_rotateValue);

   m_rotateSlider = GuiBuilder::CreateSlider(
      rotateLayout, [&](float val) { m_currentlySelectedObject->Rotate(val); },
      {glm::radians(rotationRange.first), glm::radians(rotationRange.second)}, m_currentlySelectedObject->GetSprite().GetRotation());

   m_rotateSlider->setFixedWidth(mFixedSize.x() / 2);
   m_transformSection->AddWidget(m_rotateSlider);
   m_transformSection->AddWidget(GuiBuilder::CreateBlankSpace(rotateLayout));

   // SCALE
   const auto scaleRange = std::make_pair(1, 3);
   const auto scaleValue =
      m_currentlySelectedObject->GetSprite().GetScale() + m_currentlySelectedObject->GetSprite().GetUniformScaleValue();

   auto scaleLayout = GuiBuilder::CreateLayout(this, GuiBuilder::LayoutType::GRID);

   m_transformSection->AddWidget(GuiBuilder::CreateLabel(scaleLayout, "Scale"));

   m_scaleUniformSlider = GuiBuilder::CreateSlider(
      scaleLayout,
      [&](float val) {
         m_currentlySelectedObject->GetSprite().ScaleUniformly(val);

         const auto scaleValue =
            m_currentlySelectedObject->GetSprite().GetScale() + m_currentlySelectedObject->GetSprite().GetUniformScaleValue();
      },
      {0, 2}, m_currentlySelectedObject->GetSprite().GetUniformScaleValue());

   m_scaleUniformSlider->setFixedWidth(2 * mFixedSize.x() / 3);
   m_transformSection->AddWidget(m_scaleUniformSlider);

   auto scaleLayout2 =
      GuiBuilder::CreateLayout(this, GuiBuilder::LayoutType::GRID, nanogui::Orientation::Horizontal, 3, nanogui::Alignment::Middle);

   m_transformSection->AddWidget(GuiBuilder::CreateLabel(scaleLayout2, "X"));
   m_scaleXValue = GuiBuilder::CreateFloatingPointBox(scaleLayout2, scaleValue.x, scaleRange, [&](const std::string& val) {
      const auto currentScaleValY = m_currentlySelectedObject->GetSprite().GetScale().y;
      m_currentlySelectedObject->Scale({std::stof(val), currentScaleValY});
      return true;
   });

   m_scaleXValue->setFixedWidth(mFixedSize.x() / 5);
   m_transformSection->AddWidget(m_scaleXValue);

   m_scaleXSlider = GuiBuilder::CreateSlider(
      scaleLayout2,
      [&](float val) {
         const auto currentScaleValY = m_currentlySelectedObject->GetSprite().GetScale().y;
         m_currentlySelectedObject->Scale({val, currentScaleValY});
         return true;
      },
      scaleRange, scaleValue.x);

   m_scaleXSlider->setFixedWidth(mFixedSize.x() / 3);
   m_transformSection->AddWidget(m_scaleXSlider);

   m_transformSection->AddWidget(GuiBuilder::CreateLabel(scaleLayout2, "Y"));
   m_scaleYValue = GuiBuilder::CreateFloatingPointBox(scaleLayout2, scaleValue.y, scaleRange, [&](const std::string& val) {
      const auto currentScaleValX = m_currentlySelectedObject->GetSprite().GetScale().x;
      m_currentlySelectedObject->Scale({currentScaleValX, std::stof(val)});
      return true;
   });

   m_scaleYValue->setFixedWidth(mFixedSize.x() / 5);
   m_transformSection->AddWidget(m_scaleYValue);

   m_scaleYSlider = GuiBuilder::CreateSlider(
      scaleLayout2,
      [&](float val) {
         const auto currentScaleValX = m_currentlySelectedObject->GetSprite().GetScale().x;
         m_currentlySelectedObject->Scale({currentScaleValX, val});
         return true;
      },
      scaleRange, scaleValue.y);

   m_scaleYSlider->setFixedWidth(mFixedSize.x() / 3);
   m_transformSection->AddWidget(m_scaleYSlider);

   m_transformSection->AddWidget(GuiBuilder::CreateBlankSpace(scaleLayout2));
}

void
GameObjectWindow::UpdateTransformSection()
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
GameObjectWindow::CreateShaderSection()
{
   m_shaderSection = GuiBuilder::CreateSection(this, "Shader");

   const auto textureName = m_currentlySelectedObject->GetSprite().GetTextureName();

   auto layout = GuiBuilder::CreateLayout(this, GuiBuilder::LayoutType::GRID);

   m_shaderSection->AddWidget(GuiBuilder::CreateLabel(layout, "Texture"));
   m_textureButton = GuiBuilder::CreateButton(layout, textureName, m_textureChangeCallback);

   m_shaderSection->AddWidget(m_textureButton);

   m_textureView =
      GuiBuilder::CreateImageView(GuiBuilder::CreateLayout(this, GuiBuilder::LayoutType::GRID, nanogui::Orientation::Horizontal, 1),
                                  m_currentlySelectedObject->GetSprite().GetTexture().Create(), {200, 200});
   m_shaderSection->AddWidget(m_textureView);


   m_shaderSection->AddWidget(GuiBuilder::CreateBlankSpace(layout));
}

void
GameObjectWindow::UpdateShaderSection()
{
   const auto currentlySelectedTexture = m_currentlySelectedObject->GetSprite().GetTexture().GetName();

   if (currentlySelectedTexture != m_textureButton->caption())
   {
      m_textureButton->setCaption(currentlySelectedTexture);
      m_textureView->bindImage(m_currentlySelectedObject->GetSprite().GetTexture().Create());
   }
}

void
GameObjectWindow::CreateAnimationSection()
{
   bool active = m_currentlySelectedObject->GetType() != GameObject::TYPE::PLAYER;
   m_animationSection = GuiBuilder::CreateSection(this, "Animation", active);

   auto animationTypeLayout =
      GuiBuilder::CreateLayout(this, GuiBuilder::LayoutType::GRID, nanogui::Orientation::Horizontal, 3, nanogui::Alignment::Middle, 15, 10);

   m_animationSection->AddWidget(GuiBuilder::CreateLabel(animationTypeLayout, "Type"));

   const auto animatablePtr = std::dynamic_pointer_cast< Animatable >(m_currentlySelectedObject);

   m_loopAnimationButton = GuiBuilder::CreateRadioButton(
      animationTypeLayout, "LOOP",
      [&]() {
         const auto animatablePtr = std::dynamic_pointer_cast< Animatable >(m_currentlySelectedObject);
         animatablePtr->SetAnimationType(Animatable::ANIMATION_TYPE::LOOP);
      },
      0, mFixedSize.x() / 3);

   m_reversalAnimationButton = GuiBuilder::CreateRadioButton(
      animationTypeLayout, "REVERSE",
      [&]() {
         const auto animatablePtr = std::dynamic_pointer_cast< Animatable >(m_currentlySelectedObject);
         animatablePtr->SetAnimationType(Animatable::ANIMATION_TYPE::REVERSABLE);
      },
      0, mFixedSize.x() / 3);

   if (animatablePtr)
   {
      const auto type = animatablePtr->GetAnimationType();

      type == Animatable::ANIMATION_TYPE::LOOP ? m_loopAnimationButton->setPushed(true) : m_reversalAnimationButton->setPushed(true);
   }

   m_animationSection->AddWidget(m_loopAnimationButton);
   m_animationSection->AddWidget(m_reversalAnimationButton);

   m_showAnimationSteps = GuiBuilder::CreateCheckBox(
      GuiBuilder::CreateLayout(this, GuiBuilder::LayoutType::GRID, nanogui::Orientation::Horizontal, 1, nanogui::Alignment::Middle, 5, 20),
      [&](bool choice) { m_parent.SetRenderAnimationPoints(choice); }, "Animation steps visible");
   m_animationSection->AddWidget(m_showAnimationSteps);

   m_lockAnimationSteps = GuiBuilder::CreateCheckBox(
      GuiBuilder::CreateLayout(this, GuiBuilder::LayoutType::GRID, nanogui::Orientation::Horizontal, 1, nanogui::Alignment::Middle, 5, 20),
      [&](bool choice) { m_parent.SetLockAnimationPoints(choice); }, "Lock animation steps");
   m_animationSection->AddWidget(m_lockAnimationSteps);

   m_animationSection->AddWidget(GuiBuilder::CreateLabel(
      GuiBuilder::CreateLayout(this, GuiBuilder::LayoutType::GRID, nanogui::Orientation::Horizontal, 1, nanogui::Alignment::Middle, 5, 20),
      "Animation Steps"));

   m_animationStepsLayout = GuiBuilder::CreateLayout(this, GuiBuilder::LayoutType::GRID, nanogui::Orientation::Horizontal, 4);

   m_animationSection->AddWidget(GuiBuilder::CreateLabel(m_animationStepsLayout, "X"));
   m_animationSection->AddWidget(GuiBuilder::CreateLabel(m_animationStepsLayout, "Y"));
   m_animationSection->AddWidget(GuiBuilder::CreateLabel(m_animationStepsLayout, "Rotation"));
   m_animationSection->AddWidget(GuiBuilder::CreateLabel(m_animationStepsLayout, "Time"));

   CreateAnimationSteps();

   auto animateLayout =
      GuiBuilder::CreateLayout(this, GuiBuilder::LayoutType::GRID, nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle);

   m_animateButton = GuiBuilder::CreateButton(
      animateLayout, "Animate", [&]() { m_parent.ToggleAnimateObject(); }, 0, mFixedSize.x() / 3);

   m_animationTimeSlider = GuiBuilder::CreateSlider(
      animateLayout, [&](float value) {}, {0.0f, 5.0f}, 0.0f, mFixedSize.x() / 2);

   m_animationSection->AddWidget(animateLayout);
   m_animationSection->AddWidget(m_animationTimeSlider);
   m_animationSection->AddWidget(m_animateButton);
}

void
GameObjectWindow::UpdateAnimationSection()
{
   const auto animatablePtr = std::dynamic_pointer_cast< Animatable >(m_currentlySelectedObject);

   if (animatablePtr && m_parent.IsObjectAnimated())
   {
      m_animationTimeSlider->setRange({0, animatablePtr->GetAnimationDuration().count()});
      m_animationTimeSlider->setValue(m_animationTimeSlider->value() + static_cast<float>(m_parent.GetDeltaTime().count())/1000.0f);

      if (m_animationTimeSlider->value() >= m_animationTimeSlider->range().second)
      {
         m_animationTimeSlider->setValue(0);
      }
   }
}

void
GameObjectWindow::ClearAnimationSteps()
{
   for (auto& animationPoint : m_animationSteps)
   {
      m_animationStepsLayout->removeChild(animationPoint.m_xPos);
      m_animationStepsLayout->removeChild(animationPoint.m_yPos);
      m_animationStepsLayout->removeChild(animationPoint.m_rotation);
      m_animationStepsLayout->removeChild(animationPoint.m_time);

      m_animationSection->RemoveWidget(animationPoint.m_xPos);
      m_animationSection->RemoveWidget(animationPoint.m_yPos);
      m_animationSection->RemoveWidget(animationPoint.m_rotation);
      m_animationSection->RemoveWidget(animationPoint.m_time);

      m_objects.erase(
         std::find_if(m_objects.begin(), m_objects.end(), [animationPoint](auto& object) { return object->GetID() == animationPoint.id; }));
   }

   m_animationSteps.clear();
}

void
GameObjectWindow::CreateAnimationSteps()
{
   const auto animatablePtr = std::dynamic_pointer_cast< Animatable >(m_currentlySelectedObject);

   if (animatablePtr)
   {
      const auto fixtedWidth = mFixedSize.x() / 5;
      const auto animationStepRange = std::make_pair(-100, 100);
      const auto animationSteps = animatablePtr->GetAnimationKeypoints();

      for (auto& point : animationSteps)
      {
         auto xValue = GuiBuilder::CreateFloatingPointBox(m_animationStepsLayout, point->m_end.x, animationStepRange,
                                                          [&](const std::string& val) { return true; }, {fixtedWidth, 0});

         auto yValue = GuiBuilder::CreateFloatingPointBox(m_animationStepsLayout, point->m_end.y, animationStepRange,
                                                          [&](const std::string& val) { return true; }, {fixtedWidth, 0});

         auto rotation = GuiBuilder::CreateFloatingPointBox(m_animationStepsLayout, 0.0f, animationStepRange,
                                                            [&](const std::string& val) { return true; }, {fixtedWidth, 0});

         auto time = GuiBuilder::CreateFloatingPointBox(m_animationStepsLayout, point->m_timeDuration.count(), animationStepRange,
                                                        [&](const std::string& val) { return true; }, {fixtedWidth, 0});

         m_animationSteps.push_back({point->GetID(), xValue, yValue, rotation, time});

         m_animationSection->AddWidget(xValue);
         m_animationSection->AddWidget(yValue);
         m_animationSection->AddWidget(rotation);
         m_animationSection->AddWidget(time);

         if (m_objects.end() == std::find(m_objects.begin(), m_objects.end(), point))
         {
            m_objects.push_back(point);
         }
      }

      if (m_animationTimeSlider)
      {
         m_animationTimeSlider->setRange({0.0f, animatablePtr->GetAnimationDuration().count()});
      }
   }
}