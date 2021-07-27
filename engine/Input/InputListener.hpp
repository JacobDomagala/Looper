#pragma once

#include "Event.hpp"

namespace dgame {

class InputListener
{
 public:
   virtual ~InputListener() = default;

   virtual void
   KeyCallback(const KeyEvent&)
   {
   }

   virtual void
   MouseButtonCallback(const MouseButtonEvent&)
   {
   }

   virtual void
   CursorPositionCallback(const CursorPositionEvent&)
   {
   }

   virtual void
   MouseScrollCallback(const MouseScrollEvent&)
   {
   }
};

} // namespace dgame
