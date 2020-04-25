#include "Animatable.hpp"
#include "Common.hpp"

Animatable::Animatable(ANIMATION_TYPE type)
{
   m_type = type;

   ResetAnimation();
}

void
Animatable::SetAnimationType(ANIMATION_TYPE type)
{
   m_type = type;
}

Animatable::ANIMATION_TYPE
Animatable::GetAnimationType()
{
   return m_type;
}

glm::vec2
Animatable::Animate(Timer::milliseconds updateTime)
{
   auto animationValue = glm::vec2();

   const auto currentAnimationPoint = *m_currentAnimationState.m_currentAnimationPoint;
   const auto animationDurationMs = Timer::ConvertToMs(currentAnimationPoint->m_timeDuration);
   const auto numOfSteps = animationDurationMs.count() / updateTime.count();

   auto currentDestination = currentAnimationPoint->m_destination;
   auto currentAnimationStep = currentDestination / glm::vec2(numOfSteps, numOfSteps);

   // Forward animation
   if (m_type == ANIMATION_TYPE::LOOP || !m_currentAnimationState.m_isReverse)
   {
      auto nextStep = m_currentAnimationState.m_currentAnimationPosition + currentAnimationStep;
      m_currentAnimationState.m_currentAnimationStep++;

      if (m_currentAnimationState.m_currentAnimationStep < numOfSteps)
      {
         m_currentAnimationState.m_currentAnimationPosition += currentAnimationStep;
         animationValue = currentAnimationStep;
      }
      else
      {
         ++m_currentAnimationState.m_currentAnimationPoint;

         if (m_currentAnimationState.m_currentAnimationPoint == m_animationPoints.end())
         {
            if (m_type == ANIMATION_TYPE::LOOP)
            {
               m_currentAnimationState.m_currentAnimationPoint = m_animationPoints.begin();
            }
            else
            {
               m_currentAnimationState.m_isReverse = true;
               --m_currentAnimationState.m_currentAnimationPoint;
            }
         }

         m_currentAnimationState.m_currentAnimationPosition = glm::vec2();
         m_currentAnimationState.m_currentAnimationStep = 0;
      }
   }
   else // reverse
   {
      currentDestination *= glm::vec2(-1.0f);
      currentAnimationStep *= -1.0f;

      auto nextStep = m_currentAnimationState.m_currentAnimationPosition + currentAnimationStep;
      m_currentAnimationState.m_currentAnimationStep++;

      if (m_currentAnimationState.m_currentAnimationStep < numOfSteps)
      {
         m_currentAnimationState.m_currentAnimationPosition += currentAnimationStep;
         animationValue = currentAnimationStep;
      }
      else
      {
         if (m_currentAnimationState.m_currentAnimationPoint == m_animationPoints.begin())
         {
            m_currentAnimationState.m_isReverse = false;
            m_currentAnimationState.m_currentAnimationPoint = m_animationPoints.begin();
         }
         else
         {
            --m_currentAnimationState.m_currentAnimationPoint;
         }

         m_currentAnimationState.m_currentAnimationPosition = glm::vec2();
         m_currentAnimationState.m_currentAnimationStep = 0;
      }
   }

   return animationValue;
}

void
Animatable::AddAnimationNode(std::shared_ptr< AnimationPoint > newAnimationPoint)
{
   m_animationPoints.push_back(newAnimationPoint);
}

void
Animatable::UpdateAnimationNode(std::shared_ptr< AnimationPoint > updatedAnimationPoint)
{
   auto updatedPointIt =
      std::find_if(m_animationPoints.begin(), m_animationPoints.end(), [updatedAnimationPoint](const auto animationPoint) {
         return animationPoint->GetID() == updatedAnimationPoint->GetID();
      });

   if (updatedPointIt != m_animationPoints.end())
   {
      (*updatedPointIt)->m_destination = updatedAnimationPoint->m_destination;
      (*updatedPointIt)->m_timeDuration = updatedAnimationPoint->m_timeDuration;
   }

   UpdateNodes();
}

void
Animatable::SetAnimationKeypoints(AnimationPoint::vector&& keypoints)
{
   m_animationPoints = keypoints;
}

AnimationPoint::vector
Animatable::GetAnimationKeypoints()
{
   return m_animationPoints;
}

Timer::seconds
Animatable::GetAnimationDuration() const
{
   auto totalDuration = Timer::seconds(0);

   for (auto& animationPoint : m_animationPoints)
   {
      totalDuration += animationPoint->m_timeDuration;
   }

   return totalDuration;
}

void
Animatable::Update(bool isReverse)
{
   if (isReverse)
   {
      m_currentAnimationState = m_statesQueue.back();
      m_statesQueue.pop_back();
   }
   else
   {
      m_statesQueue.push_back(m_currentAnimationState);
      if (m_statesQueue.size() >= NUM_FRAMES_TO_SAVE)
      {
         m_statesQueue.pop_front();
      }
   }
}

void
Animatable::UpdateNodes()
{
}

void
Animatable::ResetAnimation()
{
   m_currentAnimationState.m_currentAnimationPoint = m_animationPoints.begin();
   m_currentAnimationState.m_currentAnimationPosition = glm::vec2(0.0f, 0.0f);
   m_currentAnimationState.m_isReverse = false;
}

void
Animatable::RenderAnimationSteps(bool choice)
{
   m_renderAnimationSteps = choice;
}

bool
Animatable::GetRenderAnimationSteps()
{
   return m_renderAnimationSteps;
}

void
Animatable::LockAnimationSteps(bool lock)
{
   m_lockAnimationSteps = lock;
}

bool
Animatable::GetLockAnimationSteps()
{
   return m_lockAnimationSteps;
}