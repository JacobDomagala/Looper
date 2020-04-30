#pragma once

#include "Object.hpp"
#include "Section.hpp"

#include <string>
#include <vector>

class GameObject;
class Editor;

class GameObjectShaderSection : public Section
{
 public:
   GameObjectShaderSection(nanogui::Widget* parent, Editor& editor, bool activeByDefault = true);

   void
   Update();

   void
   Create(std::shared_ptr< GameObject > object);

   void
   ObjectUpdated(int ID);

   void
   GameObjectSelected(std::shared_ptr< GameObject > selectedGameObject);

 private:
   nanogui::Button* m_textureButton = nullptr;
   nanogui::ImageView* m_textureView = nullptr;
   nanogui::Button* m_shaderButton = nullptr;

   std::shared_ptr< GameObject > m_currentlySelectedObject;
   ::Object::VectorPtr m_objects;

   std::function< void() > m_textureChangeCallback;
};
