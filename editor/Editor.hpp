#pragma once

#include "Game.hpp"

#include "Level.hpp"
#include "Logger.hpp"
#include "Player.hpp"
#include "Timer.hpp"
#include "Gui.hpp"

#include <nanogui/screen.h>
#include <glm/matrix.hpp>

class Player;
class Window;

class Editor : public nanogui::Screen
{
 public:
   Editor();
   ~Editor();

   void
   draw(NVGcontext* ctx) override;
   void
   drawAll() override;
   void
   drawContents() override;

   void
   LoadLevel(const std::string& levelName);

   void
   InitGLFW();

   void
   HandleInput();

   void
   PlayLevel();

 private:
   Logger m_logger = Logger("Editor");
   
   Game m_game;
   std::unique_ptr< Player > m_player = nullptr;
   glm::vec2 m_playerPosition;

   // string -> file name in which level is stored (used for testing level in game)
   // level -> currently active level in editor
   std::pair<std::string, Level> m_currentLevel;
   
   bool m_levelLoaded = false;
   std::unique_ptr< byte_vec4 > m_collision = nullptr;
   
   // framebuffer for first pass
   Framebuffer m_frameBuffer;
   
   // projection matrix for OpenGL
   glm::mat4 m_projectionMatrix;

   InputManager m_inputManager;
   Gui m_gui;
};
