#pragma once

#include <cstdint>

namespace looper {

struct Event
{
   enum class EventType
   {
      KEY,
      CHAR,
      MOUSE_BUTTON,
      MOUSE_CURSOR,
      MOUSE_SCROLL,
      WINDOW_FOCUS
   };

   explicit Event(EventType t) : type_(t)
   {
   }

   EventType type_;
   bool handled_ = false;
};

struct KeyEvent : public Event
{
   KeyEvent(int32_t key, int32_t scanCode, int32_t action, int32_t mods)
      : Event(EventType::KEY), key_(key), scanCode_(scanCode), action_(action), mods_(mods)
   {
   }

   int32_t key_;
   int32_t scanCode_;
   int32_t action_;
   int32_t mods_;
};

struct CharEvent : public Event
{
   explicit CharEvent(uint32_t key) : Event(EventType::CHAR), key_(key)
   {
   }

   uint32_t key_;
};

struct MouseButtonEvent : public Event
{
   MouseButtonEvent(int32_t button, int32_t action, int32_t mods)
      : Event(EventType::MOUSE_BUTTON), button_(button), action_(action), mods_(mods)
   {
   }

   int32_t button_;
   int32_t action_;
   int32_t mods_;
};

struct CursorPositionEvent : public Event
{
   CursorPositionEvent(double x, double y) : Event(EventType::MOUSE_CURSOR), xPos_(x), yPos_(y)
   {
   }

   double xPos_;
   double yPos_;
};

struct MouseScrollEvent : public Event
{
   MouseScrollEvent(double xOffset, double yOffset)
      : Event(EventType::MOUSE_SCROLL), xOffset_(xOffset), yOffset_(yOffset)
   {
   }

   double xOffset_;
   double yOffset_;
};

struct WindowFocusEvent : public Event
{
   explicit WindowFocusEvent(int focus) : Event(EventType::WINDOW_FOCUS), focus_(focus)
   {
   }

   int focus_;
};

} // namespace looper