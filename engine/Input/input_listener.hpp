#pragma once

#include "event.hpp"

namespace looper {

// NOLINTNEXTLINE
class InputListener
{
 public:
   virtual ~InputListener() = default;

   virtual void
   KeyCallback(const KeyEvent& /*event*/)
   {
   }

   virtual void
   MouseButtonCallback(const MouseButtonEvent& /*event*/)
   {
   }

   virtual void
   CursorPositionCallback(const CursorPositionEvent& /*event*/)
   {
   }

   virtual void
   MouseScrollCallback(const MouseScrollEvent& /*event*/)
   {
   }
};

} // namespace looper
