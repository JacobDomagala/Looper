#pragma once

#include <functional>
#include <nanogui/window.h>

namespace nanogui {
template < typename T > class IntBox;
}

class Editor;
class GameObject;
class Section;

class GameObjectWindow : public nanogui::Window
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
   GameObjectWindow(Editor& parent);
   ~GameObjectWindow() = default;

   void
   Update();

   void
   GameObjectSelected(std::shared_ptr< GameObject > selectedGameObject);

   void
   GameObjectUnselected();

   void
   AnimationPointSelected(int id);

   void
   AnimationPointUnselected();

 private:
   void
   CreateGeneralSection();

   void
   UpdateGeneralSection();

   void
   CreateTransformSection();

   void
   UpdateTransformSection();

   void
   CreateShaderSection();

   void
   UpdateShaderSection();

   void
   CreateAnimationSection();

   void
   UpdateAnimationSection();

   void
   ClearAnimationSteps();

   void
   CreateAnimationSteps();

 private:
   // GENERAL SECTION
   Section* m_generalSection = nullptr;
   nanogui::TextBox* m_width = nullptr;
   nanogui::TextBox* m_height = nullptr;
   nanogui::TextBox* m_mapPositionX = nullptr;
   nanogui::TextBox* m_mapPositionY = nullptr;
   nanogui::TextBox* m_name = nullptr;
   nanogui::TextBox* m_type = nullptr;

   // TRANSFORM SECTION
   Section* m_transformSection = nullptr;
   nanogui::Slider* m_rotateSlider = nullptr;
   nanogui::IntBox< int32_t >* m_rotateValue = nullptr;
   nanogui::Slider* m_scaleUniformSlider = nullptr;
   nanogui::TextBox* m_scaleXValue = nullptr;
   nanogui::Slider* m_scaleXSlider = nullptr;
   nanogui::TextBox* m_scaleYValue = nullptr;
   nanogui::Slider* m_scaleYSlider = nullptr;

   // SHADER SECTION
   Section* m_shaderSection = nullptr;
   nanogui::Button* m_textureButton = nullptr;
   nanogui::ImageView* m_textureView = nullptr;
   nanogui::Button* m_shaderButton = nullptr;
   nanogui::CheckBox* m_showCollision = nullptr;

   // ANIMATE SECTION
   Section* m_animationSection = nullptr;
   nanogui::Button* m_loopAnimationButton = nullptr;
   nanogui::Button* m_reversalAnimationButton = nullptr;
   nanogui::CheckBox* m_showAnimationSteps = nullptr;
   nanogui::CheckBox* m_lockAnimationSteps = nullptr;
   nanogui::Widget* m_animationStepsLayout = nullptr;
   std::vector< AnimationPoint > m_animationSteps;
   nanogui::Button* m_animateButton = nullptr;
   nanogui::Slider* m_animationTimeSlider = nullptr;

   Editor& m_parent;

   bool m_created = false;

   std::shared_ptr< GameObject > m_currentlySelectedObject;

   std::function< void() > m_textureChangeCallback;
   std::function< void() > m_nameChangeCallback;
};
