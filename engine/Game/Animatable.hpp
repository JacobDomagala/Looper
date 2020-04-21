#pragma once

#include "Timer.hpp"

#include <deque>
#include <glm/glm.hpp>
#include <list>

class Animatable
{
 public:
   enum class ANIMATION_TYPE
   {
      LOOP,
      REVERSABLE
   };

   struct AnimationPoint
   {
      int ID;
      Timer::seconds m_timeDuration;
      glm::vec2 m_destination;
   };

   using AnimationPoints = std::list< AnimationPoint >;

 public:
   Animatable(ANIMATION_TYPE type);
   
   void SetAnimationType(ANIMATION_TYPE);
   
   ANIMATION_TYPE GetAnimationType();

   glm::vec2
   Animate(Timer::milliseconds updateTime);

   void
   AddAnimationNode(const AnimationPoint& pathNodeMapPosition);

   void
   UpdateAnimationNode(const AnimationPoint& pathNodeMapPosition);

   void
   SetAnimationKeypoints(std::list< AnimationPoint >&& keypoints);

   std::list< AnimationPoint >
   GetAnimationKeypoints();

   Timer::seconds
   GetAnimationDuration() const;

   void
   Update(bool isReverse);

 protected:
   void
   UpdateNodes();

   void
   ResetAnimation();

   struct AnimationState
   {
      std::list< AnimationPoint >::iterator m_currentAnimationPoint;
      glm::vec2 m_currentAnimationPosition{0.0f, 0.0f};
      uint32_t m_currentAnimationStep = 0;
      bool m_isReverse = false;
   };

   std::deque< AnimationState > m_statesQueue;
   AnimationState m_currentAnimationState;

   std::list< AnimationPoint > m_animationPoints;
   ANIMATION_TYPE m_type = ANIMATION_TYPE::LOOP;
};
