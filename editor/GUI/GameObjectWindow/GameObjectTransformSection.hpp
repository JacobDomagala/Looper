#pragma once

#include "Object.hpp"
#include "Section.hpp"

#include <string>
#include <vector>

namespace nanogui {
template < typename T > class IntBox;
}

class GameObject;
class Editor;

class GameObjectTransformSection : public Section
{
 public:
   GameObjectTransformSection(nanogui::Widget* parent, Editor& editor, bool activeByDefault = true);

   void
   Update();

   void
   Create(std::shared_ptr< GameObject > selectedGameObject);

   void
   ObjectUpdated(int ID);

   void
   GameObjectSelected(std::shared_ptr< GameObject > selectedGameObject);

 private:
   nanogui::Slider* m_rotateSlider = nullptr;
   nanogui::IntBox< int32_t >* m_rotateValue = nullptr;
   nanogui::Slider* m_scaleUniformSlider = nullptr;
   nanogui::TextBox* m_scaleXValue = nullptr;
   nanogui::Slider* m_scaleXSlider = nullptr;
   nanogui::TextBox* m_scaleYValue = nullptr;
   nanogui::Slider* m_scaleYSlider = nullptr;

   std::shared_ptr< GameObject > m_currentlySelectedObject;
   ::Object::VectorPtr m_objects;
};
