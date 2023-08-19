#pragma once

#include "event.hpp"

namespace looper {

// NOLINTNEXTLINE
class InputListener
{
 public:
   virtual ~InputListener() = default;

   virtual void
   KeyCallback(KeyEvent& /*event*/)
   {
   }

   virtual void
   CharCallback(CharEvent& /*event*/)
   {
   }

   virtual void
   MouseButtonCallback(MouseButtonEvent& /*event*/)
   {
   }

   virtual void
   CursorPositionCallback(CursorPositionEvent& /*event*/)
   {
   }

   virtual void
   MouseScrollCallback(MouseScrollEvent& /*event*/)
   {
   }

   virtual void
   WindowFocusCallback(WindowFocusEvent& /*event*/)
   {
   }
};

} // namespace looper
