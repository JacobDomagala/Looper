#pragma once

#include "Logger.hpp"
#include "Object.hpp"
#include "Timer.hpp"

#include <deque>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <vector>

struct AnimationPoint : public Object
{
   using vectorPtr = std::vector< std::shared_ptr< AnimationPoint > >;

   AnimationPoint() : Object(Object::TYPE::ANIMATION_POINT)
   {
   }

   // duration of adnimation throughout entire animation section
   Timer::seconds m_timeDuration = Timer::seconds(0);

   // duration of pause which object will make after reaching endpoint of this animation section
   Timer::seconds m_pauseDuration = Timer::seconds(0);

   glm::vec2 m_start;
   glm::vec2 m_end;

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

 public:
   Animatable(ANIMATION_TYPE type);

   void SetAnimationType(ANIMATION_TYPE);

   ANIMATION_TYPE
   GetAnimationType();

   glm::vec2
   SetAnimation(Timer::milliseconds updateTime);

   // Perform looped animation
   // Returns animate distance
   glm::vec2
   Animate(Timer::milliseconds updateTime);

   // Perform single animation route
   // Returns animate distance or empty when animation is finished
   std::optional< glm::vec2 >
   SingleAnimate(Timer::milliseconds updateTime);

   void
   AddAnimationNode(std::shared_ptr< AnimationPoint > pathNodeMapPosition);

   void
   UpdateAnimationNode(std::shared_ptr< AnimationPoint > pathNodeMapPosition);

   void
   SetAnimationKeypoints(AnimationPoint::vectorPtr&& keypoints);

   AnimationPoint::vectorPtr
   GetAnimationKeypoints();

   Timer::seconds
   GetAnimationDuration() const;

   void
   RenderAnimationSteps(bool choice);

   bool
   GetRenderAnimationSteps();

   void
   LockAnimationSteps(bool lock);

   bool
   GetLockAnimationSteps();

   void
   SetAnimationStartLocation(const glm::vec2& localPosition);

   glm::vec2
   GetAnimationStartLocation() const;

   void
   Update(bool isReverse);

 protected:
   void
   UpdateNodes();

   void
   ResetAnimation();

   struct AnimationState
   {
      AnimationPoint::vectorPtr::iterator m_currentAnimationPoint;
      glm::vec2 m_currentAnimationBegin;
      glm::vec2 m_currentAnimationEnd;
      glm::vec2 m_currentAnimationPosition;
      glm::vec2 m_currentAnimationDistance;

      bool m_isReverse = false;
      bool m_animationFinished = false;

      // time from current Animation Point start
      Timer::milliseconds m_currentTimeElapsed = Timer::milliseconds(0);

      // time from Animation start
      Timer::milliseconds m_totalTimeElapsed = Timer::milliseconds(0);
   };

   std::deque< AnimationState > m_statesQueue;
   AnimationState m_currentAnimationState;

   AnimationPoint::vectorPtr m_animationPoints;
   ANIMATION_TYPE m_type = ANIMATION_TYPE::LOOP;
   glm::vec2 m_animationStartPosition;

   bool m_renderAnimationSteps = false;
   bool m_lockAnimationSteps = false;

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
   SetCorrectAnimationPoint(Timer::milliseconds& updateTime);

   // Updates animation in current section
   // This should only be called after SetCorrectAnimationPoint
   glm::vec2
   AnimateInCurrentSection(Timer::milliseconds updateTime);

   // Iterate to next animation point and set all internal data related to it
   // For Reverse it's backward iteration, forward otherwise
   void
   UpdateAnimationPoint();

   // Calculate next animation step based on current animation point and 'updateTime'
   glm::vec2
   CalculateNextStep(Timer::milliseconds updateTime);

 private:
   Logger m_logger = Logger("Animatable");
};
