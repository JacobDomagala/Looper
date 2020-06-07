#pragma once
#include "Object.hpp"
#include "GuiBuilder.hpp"

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

class Gui
{
 public:
   Gui(Editor& parent);
   ~Gui() = default;

   void
   Init();

   void
   Update();

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
   ObjectUpdated(dgame::Object::ID ID);

   void
   AnimationPaused();

   void
   AnimationFinished();

   void
   ObjectDeleted(dgame::Object::ID ID);

 private:
   /*
    Creates left panel with following subwidgets:
    - Play currently loaded level
    - Load/Save current level
    */
   void
   CreateLeftPanel();

   Widget*
   CreatePlayButton(Widget* parent);

 private:
   Editor& m_parent;

   GameObjectWindow* m_currentGameObjectWindow = nullptr;
   EditorObjectWindow* m_currentEditorObjectWindow = nullptr;
   LevelWindow* m_levelWindow = nullptr;

   std::unordered_map< std::string, GuiWindow* > m_windows;
   std::vector< Widget* > m_levelDependentWidgets;
};

} // namespace dgame