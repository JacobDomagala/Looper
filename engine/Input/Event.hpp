#pragma once

#include <stdint.h>

namespace dgame {

struct Event
{
   bool m_handled = false;
};

struct KeyEvent : public Event
{
   KeyEvent(int32_t key, int32_t scanCode, int32_t action, int32_t mods)
      : Event(), m_key(key), m_scanCode(scanCode), m_action(action), m_mods(mods)
   {
   }

   int32_t m_key;
   int32_t m_scanCode;
   int32_t m_action;
   int32_t m_mods;
};

struct MouseButtonEvent : public Event
{
   MouseButtonEvent(int32_t button, int32_t action, int32_t mods) : Event(), m_buttton(button), m_action(action), m_mods(mods)
   {
   }

   int32_t m_buttton;
   int32_t m_action;
   int32_t m_mods;
};

struct CursorPositionEvent : public Event
{
   CursorPositionEvent(double x, double y) : Event(), m_xPos(x), m_yPos(y)
   {
   }

   double m_xPos;
   double m_yPos;
};

struct MouseScrollEvent : public Event
{
   MouseScrollEvent(double xOffset, double yOffset)
      : Event(), m_xOffset(xOffset), m_yOffset(yOffset)
   {
   }

   double m_xOffset;
   double m_yOffset;
};

} // namespace dgame