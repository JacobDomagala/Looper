#pragma once

#include "Object.hpp"
#include "Timer.hpp"

#include <deque>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

struct AnimationPoint : public Object
{
   AnimationPoint() : Object(Object::TYPE::ANIMATION_POINT)
   {
   }
   using vector = std::vector< std::shared_ptr<AnimationPoint> >;

   Timer::seconds m_timeDuration = Timer::seconds(0);
   glm::vec2 m_destination;
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
   Animate(Timer::milliseconds updateTime);

   void
   AddAnimationNode(std::shared_ptr< AnimationPoint > pathNodeMapPosition);

   void
   UpdateAnimationNode(std::shared_ptr< AnimationPoint > pathNodeMapPosition);

   void
   SetAnimationKeypoints(AnimationPoint::vector&& keypoints);

   AnimationPoint::vector
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
   Update(bool isReverse);

 protected:
   void
   UpdateNodes();

   void
   ResetAnimation();

   struct AnimationState
   {
      AnimationPoint::vector::iterator m_currentAnimationPoint;
      glm::vec2 m_currentAnimationPosition{0.0f, 0.0f};
      uint32_t m_currentAnimationStep = 0;
      bool m_isReverse = false;
   };

   std::deque< AnimationState > m_statesQueue;
   AnimationState m_currentAnimationState;

   AnimationPoint::vector m_animationPoints;
   ANIMATION_TYPE m_type = ANIMATION_TYPE::LOOP;

   bool m_renderAnimationSteps = false;
   bool m_lockAnimationSteps = false;
};
