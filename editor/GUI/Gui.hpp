#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

class Editor;
class GameObject;
class EditorObject;
class GameObjectWindow;
class EditorObjectWindow;
class LevelWindow;
class Level;

namespace nanogui {
class Window;
class Widget;
} // namespace nanogui

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

   void LevelLoaded(std::shared_ptr < Level > loadedLevel);

   void
   ObjectUpdated(int ID);

   void
   AnimationPaused();

   void
   AnimationFinished();

 private:
   /*
    Creates left panel with following subwidgets:
    - Play currently loaded level
    - Load/Save current level
    */
   void
   CreateLeftPanel();

   nanogui::Widget*
   CreatePlayButton(nanogui::Widget* parent);

 private:
   Editor& m_parent;

   GameObjectWindow* m_currentGameObjectWindow = nullptr;
   EditorObjectWindow* m_currentEditorObjectWindow = nullptr;
   LevelWindow* m_levelWindow = nullptr;

   std::unordered_map< std::string, nanogui::Window* > m_windows;
   std::vector< nanogui::Widget* > m_levelDependentWidgets;
};
