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
Animatable::SetCorrectAnimationPoint(Timer::milliseconds& updateTime)
{
   auto animationValue = glm::vec2();

   m_currentAnimationState.m_isReverse = false;
   m_currentAnimationState.m_currentAnimationPoint = m_animationPoints.begin();
   auto animationDurationMs = Timer::ConvertToMs((*m_currentAnimationState.m_currentAnimationPoint)->m_timeDuration);
   m_currentAnimationState.m_currentAnimationBegin = m_animationStartPosition;
   m_currentAnimationState.m_currentAnimationPosition = m_currentAnimationState.m_currentAnimationBegin;

   auto startPosition = m_currentAnimationState.m_currentAnimationBegin;
   auto destination = (*m_currentAnimationState.m_currentAnimationPoint)->m_end;

   if (updateTime >= animationDurationMs)
   {
      do
      {
         updateTime -= animationDurationMs;
         animationValue += destination - startPosition;

         if (m_currentAnimationState.m_isReverse)
         {
            --m_currentAnimationState.m_currentAnimationPoint;
         }
         else
         {
            ++m_currentAnimationState.m_currentAnimationPoint;
         }

         // If forward pass is finished
         if (m_currentAnimationState.m_currentAnimationPoint == m_animationPoints.end())
         {
            // For reversable animations start going back
            if (m_type == Animatable::ANIMATION_TYPE::REVERSABLE)
            {
               m_currentAnimationState.m_isReverse = true;
               --m_currentAnimationState.m_currentAnimationPoint;

               startPosition = (*m_currentAnimationState.m_currentAnimationPoint)->m_end;

               if (m_currentAnimationState.m_currentAnimationPoint != m_animationPoints.begin())
               {
                  auto previousStep = std::prev(m_currentAnimationState.m_currentAnimationPoint);
                  destination = (*previousStep)->m_end;
               }
               else
               {
                  destination = m_animationStartPosition;
               }

               animationDurationMs = Timer::ConvertToMs((*m_currentAnimationState.m_currentAnimationPoint)->m_timeDuration);
            }
            // For loop animation position on the end
            else
            {
            }
         }
         else
         {
            if (m_currentAnimationState.m_isReverse)
            {
               startPosition = (*m_currentAnimationState.m_currentAnimationPoint)->m_end;

               if (m_currentAnimationState.m_currentAnimationPoint != m_animationPoints.begin())
               {
                  auto previousStep = std::prev(m_currentAnimationState.m_currentAnimationPoint);
                  destination = (*previousStep)->m_end;
               }
               else
               {
                  destination = m_animationStartPosition;
               }
            }
            else
            {
               startPosition = destination;
               destination = (*m_currentAnimationState.m_currentAnimationPoint)->m_end;
            }

            animationDurationMs = Timer::ConvertToMs((*m_currentAnimationState.m_currentAnimationPoint)->m_timeDuration);
         }
      } while (updateTime >= animationDurationMs && m_currentAnimationState.m_currentAnimationPoint != m_animationPoints.begin());
   }

   m_currentAnimationState.m_currentAnimationBegin = startPosition;
   m_currentAnimationState.m_currentAnimationEnd = destination;

   return animationValue;
}

glm::vec2
Animatable::SetAnimation(Timer::milliseconds updateTime)
{
   auto animationValue = SetCorrectAnimationPoint(updateTime);

   const auto startPosition = m_currentAnimationState.m_currentAnimationBegin;
   const auto destination = m_currentAnimationState.m_currentAnimationEnd;
   const auto animationDurationMs = Timer::ConvertToMs((*m_currentAnimationState.m_currentAnimationPoint)->m_timeDuration);

   const auto sizeOfStep = updateTime.count() / static_cast< float >((animationDurationMs).count());

   const auto currentAnimationSectonLength = destination - startPosition;
   const auto currentAnimationStep = currentAnimationSectonLength * sizeOfStep;

   // Object position after adding animation step
   auto nextStep = m_currentAnimationState.m_currentAnimationPosition + currentAnimationStep;

   m_currentAnimationState.m_currentAnimationPosition = nextStep;
   animationValue += currentAnimationStep;

   m_currentAnimationState.m_currentAnimationDistance = currentAnimationStep;
   m_currentAnimationState.m_currentTimeElapsed = updateTime;

   return animationValue;
}

glm::vec2
Animatable::Animate(Timer::milliseconds updateTime)
{
   m_currentAnimationState.m_animationFinished = false;
   auto animationValue = glm::vec2();

   const auto currentAnimationPoint = *m_currentAnimationState.m_currentAnimationPoint;
   auto animationDurationMs = Timer::ConvertToMs(currentAnimationPoint->m_timeDuration);

   const auto numOfSteps =
      updateTime.count() / static_cast< float >((animationDurationMs - m_currentAnimationState.m_currentTimeElapsed).count());

   auto currentAnimationSectonLength = m_currentAnimationState.m_isReverse
                                          ? m_currentAnimationState.m_currentAnimationBegin - currentAnimationPoint->m_end
                                          : currentAnimationPoint->m_end - m_currentAnimationState.m_currentAnimationBegin;

   auto currentAnimationStep = (currentAnimationSectonLength - m_currentAnimationState.m_currentAnimationDistance) * numOfSteps;

   // Object position after adding animation step
   auto nextStep = m_currentAnimationState.m_currentAnimationPosition + currentAnimationStep;

   // Last position in current Animation Section
   const auto currentAnimationSectionFinalPosition =
      m_currentAnimationState.m_isReverse ? m_currentAnimationState.m_currentAnimationBegin : currentAnimationPoint->m_end;

   if (!IsPositionClose(currentAnimationSectionFinalPosition, nextStep, 0.5f))
   {
      m_currentAnimationState.m_currentAnimationPosition = nextStep;
      animationValue = currentAnimationStep;
      m_currentAnimationState.m_currentAnimationDistance += animationValue;
   }
   else if (m_type == ANIMATION_TYPE::LOOP || !m_currentAnimationState.m_isReverse)
   {
      m_logger.Log(Logger::TYPE::INFO, "Time when animation point finished "
                                          + std::to_string((m_currentAnimationState.m_currentTimeElapsed += updateTime).count()));

      m_currentAnimationState.m_currentTimeElapsed = Timer::milliseconds(0);
      m_currentAnimationState.m_currentAnimationDistance = glm::vec2(0.0f, 0.0f);

      animationValue = currentAnimationPoint->m_end - m_currentAnimationState.m_currentAnimationPosition;
      m_currentAnimationState.m_currentAnimationPosition = currentAnimationPoint->m_end;
      m_currentAnimationState.m_currentAnimationBegin = m_currentAnimationState.m_currentAnimationPosition;

      ++m_currentAnimationState.m_currentAnimationPoint;

      if (m_currentAnimationState.m_currentAnimationPoint == m_animationPoints.end())
      {
         if (m_type == ANIMATION_TYPE::LOOP)
         {
            m_currentAnimationState.m_animationFinished = true;
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
      m_logger.Log(Logger::TYPE::INFO, "Time when reverse animation point finished "
                                          + std::to_string((m_currentAnimationState.m_currentTimeElapsed += updateTime).count()));
      m_currentAnimationState.m_currentTimeElapsed = Timer::milliseconds(0);
      m_currentAnimationState.m_currentAnimationDistance = glm::vec2(0.0f, 0.0f);

      m_currentAnimationState.m_currentAnimationPosition = m_currentAnimationState.m_currentAnimationBegin;

      if (m_currentAnimationState.m_currentAnimationPoint == m_animationPoints.begin())
      {
         m_currentAnimationState.m_animationFinished = true;
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

   m_currentAnimationState.m_currentTimeElapsed += updateTime;

   return animationValue;
}

std::optional< glm::vec2 >
Animatable::SingleAnimate(Timer::milliseconds updateTime)
{
   if (m_currentAnimationState.m_animationFinished)
   {
      m_currentAnimationState.m_animationFinished = false;
      return {};
   }
   else
   {
      return {Animate(updateTime)};
   }
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
Animatable::SetAnimationKeypoints(AnimationPoint::vectorPtr&& keypoints)
{
   m_animationPoints = keypoints;
}

AnimationPoint::vectorPtr
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
      totalDuration += animationPoint->m_pauseDuration;
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