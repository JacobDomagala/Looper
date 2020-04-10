#pragma once

#include "Context.hpp"
#include "Game.hpp"
#include "Gui.hpp"
#include "Level.hpp"
#include "Logger.hpp"
#include "Player.hpp"
#include "Timer.hpp"

#include <glm/matrix.hpp>
#include <nanogui/screen.h>
#include <utility>

class Player;
class Window;

class Editor : public nanogui::Screen, public Context
{
 public:
   Editor(const glm::ivec2& screenSize);
   ~Editor();

   void
   draw(NVGcontext* ctx) override;

   void
   drawAll() override;

   void
   drawContents() override;

   void
   CreateLevel(const glm::ivec2& size);

   void
   LoadLevel(const std::string& levelName);

   void
   SaveLevel(const std::string& levelName);

   void
   InitGLFW();

   void
   HandleInput();

   bool
   scrollEvent(const nanogui::Vector2i& p, const nanogui::Vector2f& rel) override;

   void
   PlayLevel();

   glm::ivec2
   GetScreenSize();

   const glm::mat4&
   GetProjection() const override;

 private:
   Game m_game;

   // string -> file name in which level is stored (used for testing level in game)
   // level -> currently active level in editor
   std::string m_levelFileName;

   bool m_levelLoaded = false;
   std::unique_ptr< byte_vec4 > m_collision = nullptr;

   // projection matrix for OpenGL
   glm::mat4 m_projectionMatrix;
   glm::ivec2 m_screenSize;

   float m_zoomScale = 0.0f;
   float m_maxZoomIn = 1.5f;
   float m_maxZoomOut = -1.5f;

   Gui m_gui;
};
