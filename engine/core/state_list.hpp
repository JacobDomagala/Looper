#pragma once

#include "common.hpp"

namespace looper {

template < typename StateT > struct StateList
{
   uint32_t
   GetNumFrames() const
   {
      return numFrames_;
   }

   const StateT&
   PeekLastState() const
   {
      return frames_.at(lastIdx_);
   }

   const StateT&
   GetLastState()
   {
      lastIdx_ = (lastIdx_ == -1) ? lastFrame_ : lastIdx_;
      const auto& frame = frames_.at(lastIdx_ - 1);

      numFrames_--;
      return frame;
   }

   void
   PushState(const StateT& newState)
   {
      frames_.at(lastIdx_++) = newState;
      lastIdx_ = (lastIdx_ == lastFrame_) ? 0 : lastIdx_;
      numFrames_++;
   }

 private:
   std::array< StateT, NUM_FRAMES_TO_SAVE > frames_ = {};
   int32_t numFrames_ = {};
   int32_t firstIdx_ = {};
   int32_t lastIdx_ = {};
   const int32_t lastFrame_ = NUM_FRAMES_TO_SAVE - 1;
};

} // namespace looper
