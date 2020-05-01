#include "GameObjectWindow.hpp"
#include "Editor.hpp"
#include "Enemy.hpp"
#include "GameObject.hpp"
#include "Utils.hpp"

#include <nanogui/layout.h>

GameObjectWindow::GameObjectWindow(Editor& editor) : nanogui::Window(&editor, "Selected Object"), m_parent(editor)
{
}

void
GameObjectWindow::ObjectUpdated(int ID)
{
   m_generalSection->ObjectUpdated(ID);
   m_transformSection->ObjectUpdated(ID);
   m_shaderSection->ObjectUpdated(ID);
   m_animationSection->ObjectUpdated(ID);
}

void
GameObjectWindow::Update()
{
   m_generalSection->Update();
   m_transformSection->Update();
   m_animationSection->Update();
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

   // Don't create new window for newly selected object
   // Just refill its content with new object's values
   if (m_created)
   {
      m_generalSection->GameObjectSelected(m_currentlySelectedObject);
      m_transformSection->GameObjectSelected(m_currentlySelectedObject);
      m_shaderSection->GameObjectSelected(m_currentlySelectedObject);
      m_animationSection->GameObjectSelected(m_currentlySelectedObject);
   }
   else
   {
      mLayout = new nanogui::GroupLayout();

      setFixedSize(nanogui::Vector2i(300, frameBuffersize.y));

      m_generalSection = new GameObjectGeneralSection(this, m_parent);
      m_generalSection->Create(m_currentlySelectedObject);

      m_transformSection = new GameObjectTransformSection(this, m_parent);
      m_transformSection->Create(m_currentlySelectedObject);

      m_shaderSection = new GameObjectShaderSection(this, m_parent);
      m_shaderSection->Create(m_currentlySelectedObject);

      m_animationSection = new GameObjectAnimationSection(this, m_parent);
      m_animationSection->Create(m_currentlySelectedObject);

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
