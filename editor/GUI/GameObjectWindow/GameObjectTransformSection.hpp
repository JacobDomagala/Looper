#pragma once

#include "Object.hpp"
#include "GuiBuilder.hpp"
#include "Section.hpp"

#include <string>
#include <vector>

namespace dgame {

class GameObject;
class Editor;

class GameObjectTransformSection : public Section
{
 public:
   GameObjectTransformSection(Widget* parent, Editor& editor, bool activeByDefault = true);

   void
   Update();

   void
   Create(std::shared_ptr< GameObject > selectedGameObject);

   void
   ObjectUpdated(int ID);

   void
   GameObjectSelected(std::shared_ptr< GameObject > selectedGameObject);

 private:
   Slider* m_rotateSlider = nullptr;
   IntBox< int32_t >* m_rotateValue = nullptr;
   Slider* m_scaleUniformSlider = nullptr;
   TextBox* m_scaleXValue = nullptr;
   Slider* m_scaleXSlider = nullptr;
   TextBox* m_scaleYValue = nullptr;
   Slider* m_scaleYSlider = nullptr;

   std::shared_ptr< GameObject > m_currentlySelectedObject;
   dgame::Object::VectorPtr m_objects;
};

} // namespace dgame