#include "Animatable.hpp"
#include "Common.hpp"
#include "Utils.hpp"

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
   const auto numOfSteps = std::ceil(animationDurationMs.count() / static_cast< float >(updateTime.count()));

   auto currentAnimationSectonLength = m_currentAnimationState.m_isReverse
                                          ? m_currentAnimationState.m_currentAnimationBegin - currentAnimationPoint->m_end
                                          : currentAnimationPoint->m_end - m_currentAnimationState.m_currentAnimationBegin;

   auto currentAnimationStep = currentAnimationSectonLength / numOfSteps;

   auto nextStep = m_currentAnimationState.m_currentAnimationPosition + currentAnimationStep;

   if (!IsPositionClose(
          m_currentAnimationState.m_isReverse ? m_currentAnimationState.m_currentAnimationBegin : currentAnimationPoint->m_end, nextStep, 2.0f))
   {
      m_currentAnimationState.m_currentAnimationPosition = nextStep;
      animationValue = currentAnimationStep;
   }
   else if (m_type == ANIMATION_TYPE::LOOP || !m_currentAnimationState.m_isReverse)
   {
      animationValue = currentAnimationPoint->m_end - m_currentAnimationState.m_currentAnimationPosition;
      m_currentAnimationState.m_currentAnimationPosition = currentAnimationPoint->m_end;
      m_currentAnimationState.m_currentAnimationBegin = m_currentAnimationState.m_currentAnimationPosition;

      ++m_currentAnimationState.m_currentAnimationPoint;

      if (m_currentAnimationState.m_currentAnimationPoint == m_animationPoints.end())
      {
         if (m_type == ANIMATION_TYPE::LOOP)
         {
            m_currentAnimationState.m_currentAnimationPoint = m_animationPoints.begin();
            m_currentAnimationState.m_currentAnimationBegin = m_animationStartPosition;
         }
         else
         {
            m_currentAnimationState.m_isReverse = true;
            --m_currentAnimationState.m_currentAnimationPoint;
            m_currentAnimationState.m_currentAnimationBegin = (*std::prev(m_currentAnimationState.m_currentAnimationPoint))->m_end;
         }
      }
   }
   else // reverse
   {
      m_currentAnimationState.m_currentAnimationPosition = m_currentAnimationState.m_currentAnimationBegin;
      m_currentAnimationState.m_currentAnimationBegin = m_currentAnimationState.m_currentAnimationPosition;

      if (m_currentAnimationState.m_currentAnimationPoint == m_animationPoints.begin())
      {
         m_currentAnimationState.m_isReverse = false;
         m_currentAnimationState.m_currentAnimationBegin = m_animationStartPosition;
      }
      else
      {
         --m_currentAnimationState.m_currentAnimationPoint;

         if (m_currentAnimationState.m_currentAnimationPoint != m_animationPoints.begin())
         {
            m_currentAnimationState.m_currentAnimationBegin = (*std::prev(m_currentAnimationState.m_currentAnimationPoint))->m_end;
         }
         else
         {
            m_currentAnimationState.m_currentAnimationBegin = m_animationStartPosition;
         }
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
      (*updatedPointIt)->m_end = updatedAnimationPoint->m_end;
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

   return m_type == ANIMATION_TYPE::REVERSABLE ? 2 * totalDuration : totalDuration;
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