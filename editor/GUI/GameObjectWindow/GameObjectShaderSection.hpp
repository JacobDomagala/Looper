#pragma once

#include "Object.hpp"
#include "Section.hpp"

#include <string>
#include <vector>

namespace dgame {

class GameObject;
class Editor;

class GameObjectShaderSection : public Section
{
 public:
   GameObjectShaderSection(Widget* parent, Editor& editor, bool activeByDefault = true);

   void
   Update();

   void
   Create(std::shared_ptr< GameObject > object);

   void
   ObjectUpdated(dgame::Object::ID ID);

   void
   ObjectDeleted(dgame::Object::ID ID);

   void
   GameObjectSelected(std::shared_ptr< GameObject > selectedGameObject);

 private:
   Button* m_textureButton = nullptr;
   ImageView* m_textureView = nullptr;
   Button* m_shaderButton = nullptr;

   std::shared_ptr< GameObject > m_currentlySelectedObject;
   dgame::Object::VectorPtr m_objects;

   std::function< void() > m_textureChangeCallback;
};

} // namespace dgame