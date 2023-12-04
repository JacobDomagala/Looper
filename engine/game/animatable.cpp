#include "animatable.hpp"
#include "common.hpp"
#include "logger.hpp"

#include <algorithm>

namespace looper {

Animatable::Animatable(ANIMATION_TYPE type) : m_type(type)
{
   ResetAnimation();
}

void
Animatable::SetAnimationType(ANIMATION_TYPE type)
{
   m_type = type;
}

Animatable::ANIMATION_TYPE
Animatable::GetAnimationType() const
{
   return m_type;
}

void
Animatable::UpdateAnimationPoint()
{
   if (currentState_.m_isReverse
       and currentState_.m_currentAnimationPoint == m_animationPoints.begin())
   {
      return;
   }
   if (not currentState_.m_isReverse
       and currentState_.m_currentAnimationPoint == m_animationPoints.end())
   {
      return;
   }

   currentState_.m_isReverse ? --currentState_.m_currentAnimationPoint
                             : ++currentState_.m_currentAnimationPoint;

   auto updateReverseAnimation = [this]() {
      currentState_.m_currentAnimationBegin = currentState_.m_currentAnimationPoint->m_end;

      if (currentState_.m_currentAnimationPoint != m_animationPoints.begin())
      {
         auto previousStep = std::prev(currentState_.m_currentAnimationPoint);
         currentState_.m_currentAnimationEnd = previousStep->m_end;
      }
      else
      {
         currentState_.m_currentAnimationEnd = m_animationStartPosition;
      }
   };

   // If forward pass is finished
   if (currentState_.m_currentAnimationPoint == m_animationPoints.end())
   {
      // For reversable animations start going back
      if (m_type == Animatable::ANIMATION_TYPE::REVERSABLE)
      {
         currentState_.m_isReverse = true;
         --currentState_.m_currentAnimationPoint;

         updateReverseAnimation();
      }
      // For loop animation position on the end
      else
      {
         ResetAnimation();
         currentState_.m_animationFinished = true;
      }
   }
   else
   {
      if (currentState_.m_isReverse)
      {
         updateReverseAnimation();
      }
      else
      {
         currentState_.m_currentAnimationBegin = currentState_.m_currentAnimationEnd;
         currentState_.m_currentAnimationEnd = currentState_.m_currentAnimationPoint->m_end;
      }
   }
}

glm::vec2
Animatable::SetCorrectAnimationPoint(time::milliseconds& updateTime)
{
   auto animationValue = glm::vec2();

   ResetAnimation();

   auto animationDurationMs =
      time::Timer::ConvertToMs(currentState_.m_currentAnimationPoint->m_timeDuration);

   while (updateTime >= animationDurationMs)
   {
      updateTime -= animationDurationMs;
      animationValue += currentState_.m_currentAnimationEnd - currentState_.m_currentAnimationBegin;

      UpdateAnimationPoint();
      animationDurationMs =
         time::Timer::ConvertToMs(currentState_.m_currentAnimationPoint->m_timeDuration);
   }

   return animationValue;
}

glm::vec2
Animatable::CalculateNextStep(time::milliseconds updateTime) const
{
   const auto startPosition = currentState_.m_currentAnimationBegin;
   const auto destination = currentState_.m_currentAnimationEnd;
   const auto animationDurationMs =
      time::Timer::ConvertToMs(currentState_.m_currentAnimationPoint->m_timeDuration);

   const auto timeLeft =
      static_cast< float >((animationDurationMs - currentState_.m_currentTimeElapsed).count());

   // Make sure we don't divide by 0
   const auto sizeOfStep =
      (timeLeft > 0.0f) ? (static_cast< float >(updateTime.count()) / timeLeft) : 0.0f;

   const auto currentAnimationSectonLength = destination - startPosition;
   const auto currentAnimationStepSize =
      (currentAnimationSectonLength - currentState_.m_currentAnimationDistance) * sizeOfStep;

   return currentAnimationStepSize;
}

glm::vec2
Animatable::AnimateInCurrentSection(time::milliseconds updateTime)
{
   auto animationValue = CalculateNextStep(updateTime);

   // Object position after adding animation step
   currentState_.m_currentAnimationPosition += animationValue;
   currentState_.m_currentAnimationDistance += animationValue;
   currentState_.m_currentTimeElapsed += updateTime;

   return animationValue;
}

glm::vec2
Animatable::SetAnimation(time::milliseconds updateTime)
{
   auto animationValue = SetCorrectAnimationPoint(updateTime);
   animationValue += AnimateInCurrentSection(updateTime);

   return animationValue;
}

glm::vec2
Animatable::Animate(time::milliseconds updateTime)
{
   auto animateBy = glm::vec2();

   currentState_.m_animationFinished = false;
   currentState_.m_totalTimeElapsed += updateTime;

   auto currentAnimationStepSize = AnimateInCurrentSection(updateTime);
   if (currentState_.m_currentTimeElapsed < currentState_.m_currentAnimationPoint->m_timeDuration)
   {
      animateBy = currentAnimationStepSize;
   }
   else
   {
      currentState_.m_currentTimeElapsed = time::milliseconds(0);
      currentState_.m_currentAnimationDistance = glm::vec2(0.0f, 0.0f);

      if (currentState_.m_isReverse
          && m_animationPoints.begin() == currentState_.m_currentAnimationPoint)
      {
         ResetAnimation();
         currentState_.m_animationFinished = true;
      }
      else
      {
         UpdateAnimationPoint();
      }
   }

   return animateBy;
}

glm::vec2
Animatable::SingleAnimate(time::milliseconds updateTime)
{
   if (currentState_.m_animationFinished)
   {
      currentState_.m_animationFinished = false;
      return glm::vec2{};
   }

   return Animate(updateTime);
}

bool
Animatable::AnimationFinished() const
{
   return currentState_.m_animationFinished;
}

AnimationPoint
Animatable::CreateAnimationNode(Object::ID parentID, const glm::vec2& position)
{
   const auto nodePosition = m_animationPoints.empty()
                                ? position
                                : m_animationPoints.back().m_end + glm::vec2(20.0f, 20.0f);
   auto newNode = AnimationPoint(parentID, nodePosition, time::seconds(2));
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
                   [&updatedAnimationPoint](const auto animationPoint) {
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
      Logger::Warn("Attempting to remove non existing node with ID={}", animationID);
   }
}

void
Animatable::SetAnimationKeypoints(std::vector< AnimationPoint >&& keypoints)
{
   m_animationPoints = std::move(keypoints);
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

time::seconds
Animatable::GetAnimationDuration() const
{
   auto totalDuration = time::seconds(0);

   for (const auto& animationPoint : m_animationPoints)
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
      currentState_ = statesQueue_.GetLastState();
   }
   else
   {
      statesQueue_.PushState(currentState_);
   }
}

void
Animatable::UpdateNodes()
{
}

void
Animatable::ResetAnimation()
{
   currentState_.m_currentAnimationPoint = m_animationPoints.begin();
   currentState_.m_currentAnimationBegin = m_animationStartPosition;
   currentState_.m_currentAnimationEnd = m_animationPoints.empty()
                                            ? m_animationStartPosition
                                            : currentState_.m_currentAnimationPoint->m_end;
   currentState_.m_currentAnimationPosition = m_animationStartPosition;
   currentState_.m_currentAnimationDistance = glm::vec2(0.0f, 0.0f);
   currentState_.m_isReverse = false;
   currentState_.m_animationFinished = false;
   currentState_.m_currentTimeElapsed = time::milliseconds(0);
   currentState_.m_totalTimeElapsed = time::milliseconds(0);
}

time::milliseconds
Animatable::GetTotalTimeElapsed() const
{
   return currentState_.m_totalTimeElapsed;
}

void
Animatable::UpdateAnimationData()
{
   if (currentState_.m_isReverse)
   {
      if (currentState_.m_currentAnimationPoint != m_animationPoints.begin())
      {
         currentState_.m_currentAnimationEnd =
            std::prev(currentState_.m_currentAnimationPoint)->m_end;
      }
      else
      {
         currentState_.m_currentAnimationEnd = m_animationStartPosition;
      }

      currentState_.m_currentAnimationBegin = currentState_.m_currentAnimationPoint->m_end;
   }
   else
   {
      if (currentState_.m_currentAnimationPoint != m_animationPoints.begin())
      {
         currentState_.m_currentAnimationBegin =
            std::prev(currentState_.m_currentAnimationPoint)->m_end;
      }
      else
      {
         currentState_.m_currentAnimationBegin = m_animationStartPosition;
      }

      currentState_.m_currentAnimationEnd = m_animationPoints.empty()
                                               ? m_animationStartPosition
                                               : currentState_.m_currentAnimationPoint->m_end;
   }
}

void
Animatable::RenderAnimationSteps(bool choice)
{
   m_renderAnimationSteps = choice;
}

bool
Animatable::GetRenderAnimationSteps() const
{
   return m_renderAnimationSteps;
}

void
Animatable::LockAnimationSteps(bool lock)
{
   m_lockAnimationSteps = lock;
}

bool
Animatable::GetLockAnimationSteps() const
{
   return m_lockAnimationSteps;
}

void
Animatable::SetAnimationStartLocation(const glm::vec2& position)
{
   m_animationStartPosition = position;

   if (currentState_.m_currentAnimationPoint == m_animationPoints.begin())
   {
      currentState_.m_currentAnimationBegin = m_animationStartPosition;
      currentState_.m_currentAnimationPosition = m_animationStartPosition;
      currentState_.m_currentAnimationDistance = glm::vec2(0.0f, 0.0f);
      currentState_.m_currentTimeElapsed = time::milliseconds(0);
   }
}

glm::vec2
Animatable::GetAnimationStartLocation() const
{
   return m_animationStartPosition;
}

} // namespace looper
