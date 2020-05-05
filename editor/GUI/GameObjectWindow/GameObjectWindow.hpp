#pragma once

#include "Object.hpp"
#include "GameObjectAnimationSection.hpp"
#include "GameObjectGeneralSection.hpp"
#include "GameObjectShaderSection.hpp"
#include "GameObjectTransformSection.hpp"

#include <functional>

namespace dgame {

class Editor;
class GameObject;
class Section;

class GameObjectWindow : public GuiWindow
{
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
   ObjectUpdated(int ID);

 private:
   Editor& m_parent;

   GameObjectGeneralSection* m_generalSection = nullptr;
   GameObjectTransformSection* m_transformSection = nullptr;
   GameObjectShaderSection* m_shaderSection = nullptr;
   GameObjectAnimationSection* m_animationSection = nullptr;

   bool m_created = false;

   dgame::Object::VectorPtr m_objects;
   std::shared_ptr< GameObject > m_currentlySelectedObject;
};

} // namespace dgame