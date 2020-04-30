#pragma once

#include "Object.hpp"
#include "Section.hpp"

#include <string>
#include <vector>

class Animatable;
class GameObject;
class Editor;

class GameObjectAnimationSection : public Section
{
 public:
   struct AnimationPoint
   {
      int id;
      nanogui::TextBox* m_xPos = nullptr;
      nanogui::TextBox* m_yPos = nullptr;
      nanogui::TextBox* m_rotation = nullptr;
      nanogui::TextBox* m_time = nullptr;
   };

 public:
   GameObjectAnimationSection(nanogui::Widget* parent, Editor& editor, bool activeByDefault = true);

   void
   Update();

   void
   Create(std::shared_ptr< GameObject > selectedGameObject);

   void
   ObjectUpdated(int ID);

   void
   GameObjectSelected(std::shared_ptr< GameObject > selectedGameObject);

 private:
   void
   ClearAnimationSteps();

   void
   CreateAnimationSteps(const std::shared_ptr< Animatable >& animatablePtr);

 private:
   nanogui::Button* m_loopAnimationButton = nullptr;
   nanogui::Button* m_reversalAnimationButton = nullptr;
   nanogui::CheckBox* m_showAnimationSteps = nullptr;
   nanogui::CheckBox* m_lockAnimationSteps = nullptr;
   nanogui::Widget* m_animationStepsLayout = nullptr;
   std::vector< AnimationPoint > m_animationSteps;
   nanogui::Button* m_animateButton = nullptr;
   nanogui::Slider* m_animationTimeSlider = nullptr;

   std::shared_ptr< GameObject > m_currentlySelectedObject;
   ::Object::VectorPtr m_objects;

   bool m_created = false;
};
