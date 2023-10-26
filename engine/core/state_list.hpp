#pragma once

#include "common.hpp"

namespace looper {

template < typename StateT > struct StateList
{
   int32_t
   GetNumFrames() const
   {
      return numFrames_;
   }

   const StateT&
   PeekLastState() const
   {
      auto lastIdx = (lastIdx_ == 0) ? lastFrame_ : lastIdx_;
      return frames_.at(static_cast< uint32_t >(lastIdx - 1));
   }

   const StateT&
   GetLastState()
   {
      lastIdx_ = (lastIdx_ == 0) ? lastFrame_ : lastIdx_;
      const auto& frame = frames_.at(static_cast< uint32_t >(lastIdx_ - 1));

      lastIdx_--;
      numFrames_--;
      return frame;
   }

   void
   PushState(const StateT& newState)
   {
      frames_.at(static_cast< uint32_t >(lastIdx_++)) = newState;
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
