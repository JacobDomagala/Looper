#include "GameObjectAnimationSection.hpp"
#include "Animatable.hpp"
#include "Editor.hpp"
#include "Enemy.hpp"
#include "GuiBuilder.hpp"
#include "Utils.hpp"

namespace dgame {

GameObjectAnimationSection::GameObjectAnimationSection(nanogui::Widget* parent, Editor& editor, bool activeByDefault)
   : Section(parent, editor, "ANIMATION")
{
}

void
GameObjectAnimationSection::Create(std::shared_ptr< GameObject > selectedGameObject)
{
   m_currentlySelectedObject = selectedGameObject;
   const auto animatablePtr = std::dynamic_pointer_cast< Animatable >(m_currentlySelectedObject);

   if (animatablePtr)
   {
      bool active = m_currentlySelectedObject->GetType() != GameObject::TYPE::PLAYER;

      auto animationTypeLayout = GuiBuilder::CreateLayout(parent(), GuiBuilder::LayoutType::GRID, nanogui::Orientation::Horizontal, 3,
                                                          nanogui::Alignment::Middle, 15, 10);

      AddWidget(GuiBuilder::CreateLabel(animationTypeLayout, "Type"));


      m_loopAnimationButton = GuiBuilder::CreateRadioButton(
         animationTypeLayout, "LOOP", [&, animatablePtr]() { animatablePtr->SetAnimationType(Animatable::ANIMATION_TYPE::LOOP); }, 0,
         parent()->fixedSize().x() / 3);

      m_reversalAnimationButton = GuiBuilder::CreateRadioButton(
         animationTypeLayout, "REVERSE", [&, animatablePtr]() { animatablePtr->SetAnimationType(Animatable::ANIMATION_TYPE::REVERSABLE); },
         0, parent()->fixedSize().x() / 3);


      const auto type = animatablePtr->GetAnimationType();
      type == Animatable::ANIMATION_TYPE::LOOP ? m_loopAnimationButton->setPushed(true) : m_reversalAnimationButton->setPushed(true);

      AddWidget(m_loopAnimationButton);
      AddWidget(m_reversalAnimationButton);

      m_showAnimationSteps = GuiBuilder::CreateCheckBox(
         GuiBuilder::CreateLayout(parent(), GuiBuilder::LayoutType::GRID, nanogui::Orientation::Horizontal, 1, nanogui::Alignment::Middle,
                                  5, 20),
         [&](bool choice) { m_editor.SetRenderAnimationPoints(choice); }, "Animation steps visible");
      AddWidget(m_showAnimationSteps);

      m_lockAnimationSteps = GuiBuilder::CreateCheckBox(
         GuiBuilder::CreateLayout(parent(), GuiBuilder::LayoutType::GRID, nanogui::Orientation::Horizontal, 1, nanogui::Alignment::Middle,
                                  5, 20),
         [&](bool choice) { m_editor.SetLockAnimationPoints(choice); }, "Lock animation steps");
      AddWidget(m_lockAnimationSteps);

      m_showStepsButton = GuiBuilder::CreatePopupButton(parent(), "Animation Steps", nanogui::Popup::Side::Left);
      m_showStepsButton.second->setFixedWidth(500);

      m_animationStepsLayout = GuiBuilder::CreateLayout(m_showStepsButton.second, GuiBuilder::LayoutType::GRID,
                                                        nanogui::Orientation::Horizontal, 5, nanogui::Alignment::Middle, 2, 2);
      m_animationStepsLayout->setFixedWidth(500);

      AddWidget(GuiBuilder::CreateLabel(m_animationStepsLayout, "X"));
      AddWidget(GuiBuilder::CreateLabel(m_animationStepsLayout, "Y"));
      AddWidget(GuiBuilder::CreateLabel(m_animationStepsLayout, "Rotation"));
      AddWidget(GuiBuilder::CreateLabel(m_animationStepsLayout, "Time"));
      AddWidget(GuiBuilder::CreateLabel(m_animationStepsLayout, ""));

      CreateAnimationSteps(animatablePtr);

      auto animateLayout =
         GuiBuilder::CreateLayout(parent(), GuiBuilder::LayoutType::GRID, nanogui::Orientation::Horizontal, 2, nanogui::Alignment::Middle);

      m_animateButton = GuiBuilder::CreateButton(
         animateLayout, "Animate", [&]() { m_editor.ToggleAnimateObject(); }, 0, parent()->fixedSize().x() / 3);

      m_animationTimeSlider = GuiBuilder::CreateSlider(
         animateLayout, [&](float value) {}, {0.0f, 5.0f}, 0.0f, parent()->fixedSize().x() / 2);

      m_animationTimeSlider->setRange({0.0f, Timer::ConvertToMs(animatablePtr->GetAnimationDuration()).count()});
      m_animationTimeSlider->setCallback([&, animatablePtr](float value) {
         m_currentlySelectedObject->GetSprite().SetTranslateValue(
            animatablePtr->SetAnimation(Timer::milliseconds(static_cast< uint64_t >(value))));
         return true;
      });

      AddWidget(m_showStepsButton.first);
      AddWidget(m_showStepsButton.second);
      AddWidget(animateLayout);
      AddWidget(m_animationTimeSlider);
      AddWidget(m_animateButton);

      m_created = true;
   }

   setActive(m_created);
}

void
GameObjectAnimationSection::Update()
{
   const auto animatablePtr = std::dynamic_pointer_cast< Animatable >(m_currentlySelectedObject);

   if (animatablePtr)
   {
      if (m_editor.IsObjectAnimated())
      {
         m_animationTimeSlider->setRange({0, Timer::ConvertToMs(animatablePtr->GetAnimationDuration()).count()});
         m_animationTimeSlider->setValue(m_animationTimeSlider->value() + m_editor.GetDeltaTime().count());
      }
      else
      {
         // m_animationTimeSlider->setValue(0);
      }
   }
}

void
GameObjectAnimationSection::GameObjectSelected(std::shared_ptr< GameObject > selectedGameObject)
{
   m_currentlySelectedObject = selectedGameObject;

   if (m_objects.end() == std::find(m_objects.begin(), m_objects.end(), m_currentlySelectedObject))
   {
      m_objects.push_back(m_currentlySelectedObject);
   }

   const auto animatablePtr = std::dynamic_pointer_cast< Animatable >(m_currentlySelectedObject);

   if (animatablePtr)
   {
      if (m_created)
      {
         setActive(true);
         ObjectUpdated(m_currentlySelectedObject->GetID());
         ClearAnimationSteps();
         CreateAnimationSteps(animatablePtr);
      }
      else
      {
         Create(m_currentlySelectedObject);
      }
   }
   else
   {
      setActive(false);
   }
}

void
GameObjectAnimationSection::ObjectUpdated(dgame::Object::ID ID)
{
   auto object = std::find_if(m_objects.begin(), m_objects.end(), [ID](auto& object) { return object->GetID() == ID; });

   if (object != m_objects.end())
   {
      switch ((*object)->GetType())
      {
         case dgame::Object::TYPE::ANIMATION_POINT: {
            auto point = std::find_if(m_animationSteps.begin(), m_animationSteps.end(), [ID](auto& point) { return point.id == ID; });

            if (point != m_animationSteps.end())
            {
               auto animatioinPoint = std::dynamic_pointer_cast< dgame::AnimationPoint >(*object);

               point->m_xPos->setValue(CustomFloatToStr(animatioinPoint->m_end.x));
               point->m_yPos->setValue(CustomFloatToStr(animatioinPoint->m_end.y));
               point->m_rotation->setValue(CustomFloatToStr(animatioinPoint->m_rotation));
               point->m_time->setValue(CustomFloatToStr(animatioinPoint->m_timeDuration.count()));
            }
         }
         break;

         case dgame::Object::TYPE::ENEMY: {
            m_showAnimationSteps->setChecked(std::dynamic_pointer_cast< Enemy >(*object)->GetRenderAnimationSteps());
         }
         break;
      }
   }
}

void
GameObjectAnimationSection::ObjectDeleted(dgame::Object::ID ID)
{
   auto object = std::find_if(m_objects.begin(), m_objects.end(), [ID](auto& object) { return object->GetID() == ID; });

   if (object != m_objects.end())
   {
      switch ((*object)->GetType())
      {
         case dgame::Object::TYPE::ANIMATION_POINT: {
            auto animationPoint =
               std::find_if(m_animationSteps.begin(), m_animationSteps.end(), [ID](auto& point) { return point.id == ID; });

            if (animationPoint != m_animationSteps.end())
            {
               m_animationStepsLayout->removeChild(animationPoint->m_xPos);
               m_animationStepsLayout->removeChild(animationPoint->m_yPos);
               m_animationStepsLayout->removeChild(animationPoint->m_rotation);
               m_animationStepsLayout->removeChild(animationPoint->m_time);
               m_animationStepsLayout->removeChild(animationPoint->m_removePoint);

               RemoveWidget(animationPoint->m_xPos);
               RemoveWidget(animationPoint->m_yPos);
               RemoveWidget(animationPoint->m_rotation);
               RemoveWidget(animationPoint->m_time);
               RemoveWidget(animationPoint->m_removePoint);

               m_objects.erase(object);
            }
         }
         break;

         case dgame::Object::TYPE::ENEMY: {
            
         }
         break;
      }
   }
}

void
GameObjectAnimationSection::ClearAnimationSteps()
{
   for (auto& animationPoint : m_animationSteps)
   {
      m_animationStepsLayout->removeChild(animationPoint.m_xPos);
      m_animationStepsLayout->removeChild(animationPoint.m_yPos);
      m_animationStepsLayout->removeChild(animationPoint.m_rotation);
      m_animationStepsLayout->removeChild(animationPoint.m_time);
      m_animationStepsLayout->removeChild(animationPoint.m_removePoint);

      RemoveWidget(animationPoint.m_xPos);
      RemoveWidget(animationPoint.m_yPos);
      RemoveWidget(animationPoint.m_rotation);
      RemoveWidget(animationPoint.m_time);
      RemoveWidget(animationPoint.m_removePoint);

      m_objects.erase(
         std::find_if(m_objects.begin(), m_objects.end(), [animationPoint](auto& object) { return object->GetID() == animationPoint.id; }));
   }

   m_animationSteps.clear();
}

void
GameObjectAnimationSection::CreateAnimationSteps(const std::shared_ptr< Animatable >& animatablePtr)
{
   const auto fixtedWidth = parent()->fixedSize().x() / 5;
   const auto animationStepRange = std::make_pair(-100, 100);
   const auto animationSteps = animatablePtr->GetAnimationKeypoints();

   for (auto& point : animationSteps)
   {
      auto xValue = GuiBuilder::CreateFloatingPointBox(m_animationStepsLayout, point->m_end.x, animationStepRange,
                                                       [point](const std::string& val) {
                                                          point->m_end.x = std::stof(val);
                                                          return true;
                                                       },
                                                       {fixtedWidth, 0});

      auto yValue = GuiBuilder::CreateFloatingPointBox(m_animationStepsLayout, point->m_end.y, animationStepRange,
                                                       [point](const std::string& val) {
                                                          point->m_end.y = std::stof(val);
                                                          return true;
                                                       },
                                                       {fixtedWidth, 0});

      auto rotation = GuiBuilder::CreateFloatingPointBox(m_animationStepsLayout, 0.0f, animationStepRange,
                                                         [point](const std::string& val) {
                                                            point->m_rotation = std::stof(val);
                                                            return true;
                                                         },
                                                         {fixtedWidth, 0});

      auto time = GuiBuilder::CreateFloatingPointBox(m_animationStepsLayout, point->m_timeDuration.count(), animationStepRange,
                                                     [point](const std::string& val) {
                                                        point->m_timeDuration = Timer::seconds(static_cast< uint64_t >(std::stof(val)));
                                                        return true;
                                                     },
                                                     {fixtedWidth, 0});

      auto removePoint = GuiBuilder::CreateButton(
         m_animationStepsLayout, "", []() {}, ENTYPO_ICON_TRASH, fixtedWidth);

      m_animationSteps.push_back({point->GetID(), xValue, yValue, rotation, time, removePoint});

      AddWidget(xValue);
      AddWidget(yValue);
      AddWidget(rotation);
      AddWidget(time);
      AddWidget(removePoint);

      if (m_objects.end() == std::find(m_objects.begin(), m_objects.end(), point))
      {
         m_objects.push_back(point);
      }
   }

   if (m_animationTimeSlider)
   {
      m_animationTimeSlider->setRange({0.0f, Timer::ConvertToMs(animatablePtr->GetAnimationDuration()).count()});
   }
}

} // namespace dgame