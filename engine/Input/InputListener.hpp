#pragma once

#include "Event.hpp"

namespace dgame {

class InputListener
{
 public:
   virtual void
   KeyCallback(const KeyEvent& event)
   {
   }

   virtual void
   MouseButtonCallback(const MouseButtonEvent& event)
   {
   }

   virtual void
   CursorPositionCallback(const CursorPositionEvent& event)
   {
   }

   virtual void
   MouseScrollCallback(const MouseScrollEvent& event)
   {
   }
};

} // namespace dgame