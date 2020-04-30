#pragma once

#include "GameObjectAnimationSection.hpp"
#include "GameObjectGeneralSection.hpp"
#include "Object.hpp"
#include "GameObjectShaderSection.hpp"
#include "GameObjectTransformSection.hpp"

#include <functional>
#include <nanogui/window.h>

class Editor;
class GameObject;
class Section;

class GameObjectWindow : public nanogui::Window
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

   ::Object::VectorPtr m_objects;
   std::shared_ptr< GameObject > m_currentlySelectedObject;
};
