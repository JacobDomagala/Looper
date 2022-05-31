#pragma once

#include "Object.hpp"

#include <glm/glm.hpp>
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
   explicit EditorGUI(Editor& parent);

   void
   Init();

   static void
   Shutdown();

   void
   Render();

   void
   GameObjectSelected(const std::shared_ptr< GameObject >& selectedGameObject);

   void
   GameObjectUnselected();

   void
   EditorObjectSelected(const std::shared_ptr< EditorObject >& object);

   void
   EditorObjectUnselected();

   void
   LevelLoaded(const std::shared_ptr< Level >& loadedLevel);

   void
   ObjectUpdated(Object::ID ID);

   void
   ObjectDeleted(Object::ID ID);

   static bool
   IsBlockingEvents();

 private:
   void
   RenderMainPanel();
   void
   RenderLevelMenu();
   void
   RenderGameObjectMenu();

   Editor& m_parent;

   // EditorObjectWindow m_editorObjectWindow;
   std::shared_ptr< GameObject > m_currentlySelectedGameObject;
   std::shared_ptr< Level > m_currentLevel;

   glm::vec2 m_windowSize = glm::vec2{};
   float m_windowWidth = 0.0f;
   float m_toolsWindowHeight = 0.0f;
   float m_gameObjectWindowHeight = 0.0f;
   float m_levelWindowHeight = 0.0f;
   float m_debugWindowHeight = 0.0f;
   float m_debugWindowWidth = 0.0f;
};

} // namespace dgame