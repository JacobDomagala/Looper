#pragma once

#include "object.hpp"
#include "state_list.hpp"
#include "utils/time/timer.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace looper {

struct AnimationPoint : public Object
{
   explicit AnimationPoint(ID parentID) : Object(ObjectType::ANIMATION_POINT), m_parent(parentID)
   {
   }

   AnimationPoint(ID parentID, const glm::vec2& endPosition, time::seconds timeDuration)
      : Object(ObjectType::ANIMATION_POINT),
        m_parent(parentID),
        m_timeDuration(timeDuration),
        m_end(endPosition)
   {
   }

   ID m_parent = INVALID_ID;

   // duration of adnimation throughout entire animation section
   time::seconds m_timeDuration = time::seconds(0);

   // duration of pause which object will make after reaching endpoint of this animation section
   time::seconds m_pauseDuration = time::seconds(0);

   glm::vec2 m_end = glm::vec2(0.0f, 0.0f);

   float m_rotation = 0.0f;
};

class Animatable
{
 public:
   enum class ANIMATION_TYPE
   {
      LOOP,
      REVERSABLE
   };

   Animatable() = default;
   explicit Animatable(ANIMATION_TYPE type);

   void SetAnimationType(ANIMATION_TYPE);

   [[nodiscard]] ANIMATION_TYPE
   GetAnimationType() const;

   glm::vec2
   SetAnimation(time::milliseconds updateTime);

   // Perform looped animation
   // Returns animate distance
   glm::vec2
   Animate(time::milliseconds updateTime);

   // Perform single animation route
   // Returns animate distance or empty when animation is finished
   glm::vec2
   SingleAnimate(time::milliseconds updateTime);

   bool
   AnimationFinished() const;

   AnimationPoint
   CreateAnimationNode(Object::ID parentID, const glm::vec2& position = glm::vec2{});

   void
   AddAnimationNode(const AnimationPoint& newAnimationPoint);

   void
   UpdateAnimationNode(const AnimationPoint& updatedAnimationPoint);

   void
   DeleteAnimationNode(Object::ID animationID);

   void
   SetAnimationKeypoints(std::vector< AnimationPoint >&& keypoints);

   std::vector< AnimationPoint >&
   GetAnimationKeypoints();

   [[nodiscard]] const std::vector< AnimationPoint >&
   GetAnimationKeypoints() const;

   [[nodiscard]] time::seconds
   GetAnimationDuration() const;

   void
   RenderAnimationSteps(bool choice);

   [[nodiscard]] bool
   GetRenderAnimationSteps() const;

   void
   LockAnimationSteps(bool lock);

   [[nodiscard]] bool
   GetLockAnimationSteps() const;

   void
   SetAnimationStartLocation(const glm::vec2& position);

   [[nodiscard]] glm::vec2
   GetAnimationStartLocation() const;

   [[nodiscard]] time::milliseconds
   GetTotalTimeElapsed() const;

   void
   UpdateAnimation(bool isReverse);

   void
   ResetAnimation();

   void
   UpdateAnimationData();

 protected:
   void
   UpdateNodes();

   struct State
   {
      std::vector< AnimationPoint >::iterator currentAnimationPoint_;
      glm::vec2 currentAnimationBegin_;
      glm::vec2 currentAnimationEnd_;
      glm::vec2 currentAnimationPosition_;
      glm::vec2 currentAnimationDistance_;

      bool isReverse_ = false;
      bool animationFinished_ = false;

      // time from current Animation Point start
      time::milliseconds currentTimeElapsed_ = time::milliseconds(0);

      // time from Animation start
      time::milliseconds totalTimeElapsed_ = time::milliseconds(0);
   };

   StateList< State > animatableStatesQueue_;
   State animatableCurrentState_;

   std::vector< AnimationPoint > animationPoints_;
   ANIMATION_TYPE type_ = ANIMATION_TYPE::LOOP;
   glm::vec2 animationStartPosition_ = {};

   bool renderAnimationSteps_ = false;
   bool lockAnimationSteps_ = false;

 private:
   // Uses 'updateTime' value to determine which animation point is appropriate for this time value
   // Will fill internal structure 'm_currentAnimationState' with correct values
   // Returns animationDistance calculated by traversing all Animation Points using 'updateTime'
   // 'updateTime' will be changed to the remaining time in current Animation Point
   //
   // Example:
   // Animation Points:
   // 1. X=20 Y=40  Time=3s
   // 2. X=2  Y=100 Time=5s
   // 3. X=40 Y=1   Time=1s
   //
   // updateTime = 5000ms -> 5s
   //
   // Return:
   // - Current Animation Point -> (2)
   // - animationDistance -> (20,40)
   // - updateTime -> (2000ms -> 2s)
   glm::vec2
   SetCorrectAnimationPoint(time::milliseconds& updateTime);

   // Updates animation in current section
   // This should only be called after SetCorrectAnimationPoint
   glm::vec2
   AnimateInCurrentSection(time::milliseconds updateTime);

   // Iterate to next animation point and set all internal data related to it
   // For Reverse it's backward iteration, forward otherwise
   void
   UpdateAnimationPoint();

   // Calculate next animation step based on current animation point and 'updateTime'
   [[nodiscard]] glm::vec2
   CalculateNextStep(time::milliseconds updateTime) const;
};

} // namespace looper
