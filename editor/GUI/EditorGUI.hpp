#pragma once
#include "Event.hpp"
#include "Object.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace dgame {

class Editor;
class GameObject;
class EditorObject;
class GameObjectWindow;
class EditorObjectWindow;
class LevelWindow;
class Level;

class EditorGUI
{
 public:
   EditorGUI(Editor& parent);
   ~EditorGUI() = default;

   void
   Init();

   void
   Render();

   void
   GameObjectSelected(std::shared_ptr< GameObject > selectedGameObject);

   void
   GameObjectUnselected();

   void
   EditorObjectSelected(std::shared_ptr< EditorObject > object);

   void
   EditorObjectUnselected();

   void
   LevelLoaded(std::shared_ptr< Level > loadedLevel);

   void
   ObjectUpdated(Object::ID ID);

   void
   ObjectDeleted(Object::ID ID);

   bool
   OnEvent(const Event& e);

 private:
   Editor& m_parent;

   // EditorObjectWindow m_editorObjectWindow;
   std::shared_ptr< GameObject > m_currentlySelectedGameObject;
   std::shared_ptr< Level > m_currentLevel;

   float m_windowWidth;
};

} // namespace dgame