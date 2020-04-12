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

   bool
   scrollEvent(const nanogui::Vector2i& p, const nanogui::Vector2f& rel) override;

   bool
   mouseMotionEvent(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers) override;

   bool
   mouseButtonEvent(const nanogui::Vector2i& p, int button, bool down, int modifiers) override;

   bool
   mouseDragEvent(const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button, int modifiers) override;

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

   void
   PlayLevel();

   void
   ShowWireframe(int wireframeEnabled);

   void
   MainLoop() override;

   const glm::vec2&
   GetWindowSize() const override;

   const glm::mat4&
   GetProjection() const override;

   float
   GetZoomLevel() override;

 private:
   bool
   IsRunning() override;

   void
   UpdateCamera();

   Game m_game;

   std::string m_levelFileName;

   bool m_isRunning = true;
   bool m_levelLoaded = false;
   bool m_mousePressedLastUpdate = false;
   bool m_mouseDrag = false;

   glm::vec2 m_lastCursorPosition;

   std::unique_ptr< byte_vec4 > m_collision = nullptr;

   // projection matrix for OpenGL
   glm::mat4 m_projectionMatrix;
   glm::vec2 m_windowSize;

   glm::vec3 m_cameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);
   float m_zoomScale = 0.0f;
   float m_maxZoomIn = 1.5f;
   float m_maxZoomOut = -1.5f;

   float m_cameraSpeed = 400.0f;

   bool m_objectSelected = false;
   std::shared_ptr< GameObject > m_currentSelectedObject = nullptr;

   Timer m_timer;
   Gui m_gui;
};
