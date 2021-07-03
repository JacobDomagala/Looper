#include "Animatable.hpp"
#include "Common.hpp"
#include "Utils.hpp"

#include <algorithm>

namespace dgame {

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

void
Animatable::UpdateAnimationPoint()
{
   m_currentAnimationState.m_isReverse ? --m_currentAnimationState.m_currentAnimationPoint
                                       : ++m_currentAnimationState.m_currentAnimationPoint;

   auto updateReverseAnimation = [this]() {
      m_currentAnimationState.m_currentAnimationBegin =
         m_currentAnimationState.m_currentAnimationPoint->m_end;

      if (m_currentAnimationState.m_currentAnimationPoint != m_animationPoints.begin())
      {
         auto previousStep = std::prev(m_currentAnimationState.m_currentAnimationPoint);
         m_currentAnimationState.m_currentAnimationEnd = previousStep->m_end;
      }
      else
      {
         m_currentAnimationState.m_currentAnimationEnd = m_animationStartPosition;
      }
   };

   // If forward pass is finished
   if (m_currentAnimationState.m_currentAnimationPoint == m_animationPoints.end())
   {
      // For reversable animations start going back
      if (m_type == Animatable::ANIMATION_TYPE::REVERSABLE)
      {
         m_currentAnimationState.m_isReverse = true;
         --m_currentAnimationState.m_currentAnimationPoint;

         updateReverseAnimation();
      }
      // For loop animation position on the end
      else
      {
         ResetAnimation();
         m_currentAnimationState.m_animationFinished = true;
      }
   }
   else
   {
      if (m_currentAnimationState.m_isReverse)
      {
         updateReverseAnimation();
      }
      else
      {
         m_currentAnimationState.m_currentAnimationBegin =
            m_currentAnimationState.m_currentAnimationEnd;
         m_currentAnimationState.m_currentAnimationEnd =
            m_currentAnimationState.m_currentAnimationPoint->m_end;
      }
   }
}

glm::vec2
Animatable::SetCorrectAnimationPoint(Timer::milliseconds& updateTime)
{
   auto animationValue = glm::vec2();

   ResetAnimation();

   auto animationDurationMs =
      Timer::ConvertToMs(m_currentAnimationState.m_currentAnimationPoint->m_timeDuration);

   if (updateTime >= animationDurationMs)
   {
      do
      {
         updateTime -= animationDurationMs;
         animationValue += m_currentAnimationState.m_currentAnimationEnd
                           - m_currentAnimationState.m_currentAnimationBegin;

         UpdateAnimationPoint();
         animationDurationMs =
            Timer::ConvertToMs(m_currentAnimationState.m_currentAnimationPoint->m_timeDuration);
      } while (updateTime >= animationDurationMs
               && m_currentAnimationState.m_currentAnimationPoint != m_animationPoints.begin());
   }

   return animationValue;
}

glm::vec2
Animatable::CalculateNextStep(Timer::milliseconds updateTime)
{
   const auto startPosition = m_currentAnimationState.m_currentAnimationBegin;
   const auto destination = m_currentAnimationState.m_currentAnimationEnd;
   const auto animationDurationMs =
      Timer::ConvertToMs(m_currentAnimationState.m_currentAnimationPoint->m_timeDuration);

   const auto timeLeft = static_cast< float >(
      (animationDurationMs - m_currentAnimationState.m_currentTimeElapsed).count());

   // Make sure we don't divide by 0
   const auto sizeOfStep =
      (timeLeft > 0.0f) ? (static_cast< float >(updateTime.count()) / timeLeft) : 0.0f;

   const auto currentAnimationSectonLength = destination - startPosition;
   const auto currentAnimationStepSize =
      (currentAnimationSectonLength - m_currentAnimationState.m_currentAnimationDistance)
      * sizeOfStep;

   return currentAnimationStepSize;
}

glm::vec2
Animatable::AnimateInCurrentSection(Timer::milliseconds updateTime)
{
   auto animationValue = CalculateNextStep(updateTime);

   // Object position after adding animation step
   m_currentAnimationState.m_currentAnimationPosition += animationValue;
   m_currentAnimationState.m_currentAnimationDistance += animationValue;
   m_currentAnimationState.m_currentTimeElapsed += updateTime;

   return animationValue;
}

glm::vec2
Animatable::SetAnimation(Timer::milliseconds updateTime)
{
   auto animationValue = SetCorrectAnimationPoint(updateTime);
   animationValue += AnimateInCurrentSection(updateTime);

   return animationValue;
}

glm::vec2
Animatable::Animate(Timer::milliseconds updateTime)
{
   auto animateBy = glm::vec2();

   m_currentAnimationState.m_animationFinished = false;

   auto currentAnimationStepSize = AnimateInCurrentSection(updateTime);
   if (m_currentAnimationState.m_currentTimeElapsed
       < m_currentAnimationState.m_currentAnimationPoint->m_timeDuration)
   {
      animateBy = currentAnimationStepSize;
   }
   else
   {
      m_currentAnimationState.m_currentTimeElapsed = Timer::milliseconds(0);
      m_currentAnimationState.m_currentAnimationDistance = glm::vec2(0.0f, 0.0f);

      if (m_currentAnimationState.m_isReverse
          && m_animationPoints.begin() == m_currentAnimationState.m_currentAnimationPoint)
      {
         ResetAnimation();
         m_currentAnimationState.m_animationFinished = true;
      }
      else
      {
         UpdateAnimationPoint();
      }
   }

   return animateBy;
}

glm::vec2
Animatable::SingleAnimate(Timer::milliseconds updateTime)
{
   if (m_currentAnimationState.m_animationFinished)
   {
      m_currentAnimationState.m_animationFinished = false;
      return glm::vec2{};
   }
   else
   {
      return Animate(updateTime);
   }
}

AnimationPoint
Animatable::CreateAnimationNode(Object::ID parentID)
{
   const auto position = m_animationPoints.empty()
                            ? glm::vec2(0.0f, 0.0f)
                            : m_animationPoints.back().m_end + glm::vec2(10.0f, 10.0f);
   auto newNode = AnimationPoint(parentID, position, Timer::seconds(2));
   AddAnimationNode(newNode);

   return newNode;
}

void
Animatable::AddAnimationNode(const AnimationPoint& newAnimationPoint)
{
   m_animationPoints.push_back(newAnimationPoint);
}

void
Animatable::UpdateAnimationNode(const AnimationPoint& updatedAnimationPoint)
{
   auto updatedPointIt =
      std::find_if(m_animationPoints.begin(), m_animationPoints.end(),
                   [updatedAnimationPoint](const auto animationPoint) {
                      return animationPoint.GetID() == updatedAnimationPoint.GetID();
                   });

   if (updatedPointIt != m_animationPoints.end())
   {
      updatedPointIt->m_end = updatedAnimationPoint.m_end;
      updatedPointIt->m_timeDuration = updatedAnimationPoint.m_timeDuration;
   }

   UpdateNodes();
}

void
Animatable::DeleteAnimationNode(Object::ID animationID)
{
   auto it =
      std::find_if(m_animationPoints.begin(), m_animationPoints.end(),
                   [animationID](const auto& point) { return point.GetID() == animationID; });
   /*auto it = std::find(m_animationPoints.begin(), m_animationPoints.end(), pathNodeMapPosition);*/

   if (it != m_animationPoints.end())
   {
      m_animationPoints.erase(it);
   }
   else
   {
      m_logger.Log(Logger::TYPE::WARNING, "Attempting to remove non existing node with ID={}",
                   animationID);
   }
}

void
Animatable::SetAnimationKeypoints(const std::vector< AnimationPoint >& keypoints)
{
   m_animationPoints = keypoints;
   ResetAnimation();
}

std::vector< AnimationPoint >&
Animatable::GetAnimationKeypoints()
{
   return m_animationPoints;
}

const std::vector< AnimationPoint >&
Animatable::GetAnimationKeypoints() const
{
   return m_animationPoints;
}

Timer::seconds
Animatable::GetAnimationDuration() const
{
   auto totalDuration = Timer::seconds(0);

   for (auto& animationPoint : m_animationPoints)
   {
      totalDuration += animationPoint.m_timeDuration;
      totalDuration += animationPoint.m_pauseDuration;
   }

   // REVERSABLE animation takes twice as long
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
   m_currentAnimationState.m_currentAnimationBegin = m_animationStartPosition;
   m_currentAnimationState.m_currentAnimationEnd =
      m_animationPoints.empty() ? m_animationStartPosition
                                : m_currentAnimationState.m_currentAnimationPoint->m_end;
   m_currentAnimationState.m_currentAnimationPosition = m_animationStartPosition;
   m_currentAnimationState.m_currentAnimationDistance = glm::vec2(0.0f, 0.0f);
   m_currentAnimationState.m_isReverse = false;
   m_currentAnimationState.m_animationFinished = false;
   m_currentAnimationState.m_currentTimeElapsed = Timer::milliseconds(0);
   m_currentAnimationState.m_totalTimeElapsed = Timer::milliseconds(0);
}

void
Animatable::UpdateAnimationData()
{
   if (m_currentAnimationState.m_isReverse)
   {
      if (m_currentAnimationState.m_currentAnimationPoint != m_animationPoints.begin())
      {
         m_currentAnimationState.m_currentAnimationEnd =
            std::prev(m_currentAnimationState.m_currentAnimationPoint)->m_end;
      }
      else
      {
         m_currentAnimationState.m_currentAnimationEnd = m_animationStartPosition;
      }

      m_currentAnimationState.m_currentAnimationBegin =
         m_currentAnimationState.m_currentAnimationPoint->m_end;
   }
   else
   {
      if (m_currentAnimationState.m_currentAnimationPoint != m_animationPoints.begin())
      {
         m_currentAnimationState.m_currentAnimationBegin =
            std::prev(m_currentAnimationState.m_currentAnimationPoint)->m_end;
      }
      else
      {
         m_currentAnimationState.m_currentAnimationBegin = m_animationStartPosition;
      }

      m_currentAnimationState.m_currentAnimationEnd =
         m_animationPoints.empty() ? m_animationStartPosition
                                   : m_currentAnimationState.m_currentAnimationPoint->m_end;
   }
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

void
Animatable::SetAnimationStartLocation(const glm::vec2& localPosition)
{
   m_animationStartPosition = localPosition;

   if (m_currentAnimationState.m_currentAnimationPoint == m_animationPoints.begin())
   {
      m_currentAnimationState.m_currentAnimationBegin = m_animationStartPosition;
      m_currentAnimationState.m_currentAnimationPosition = m_animationStartPosition;
      m_currentAnimationState.m_currentAnimationDistance = glm::vec2(0.0f, 0.0f);
      m_currentAnimationState.m_currentTimeElapsed = Timer::milliseconds(0);
   }
}

glm::vec2
Animatable::GetAnimationStartLocation() const
{
   return m_animationStartPosition;
}

} // namespace dgame
