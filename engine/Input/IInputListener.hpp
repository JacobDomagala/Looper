#pragma once

#include <string_view>

class IInputListener
{
 public:
   // @brief Callback function called when any key is pressed
   // @param key key code
   // @param scancode
   // @param action
   // @param mods
   virtual void
   KeyCallback(int key, int scancode, int action, int mods) = 0;

   // @brief Callback function called when mouse button is pressed
   // @param key key code
   // @param scancode
   // @param action
   // @param mods
   virtual void
   MouseButtonCallback(int button, int action, int mods) = 0;

   // @brief Callback function called when mouse cursor is moved
   // @param key key code
   // @param scancode
   // @param action
   // @param mods
   virtual void
   CursorPositionCallback(double x, double y) = 0;
};
