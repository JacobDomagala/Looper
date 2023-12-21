#include "animatable.hpp"
#include "common.hpp"
#include "logger.hpp"

#include <algorithm>

namespace looper {

Animatable::Animatable(ANIMATION_TYPE type) : type_(type)
{
   ResetAnimation();
}

void
Animatable::SetAnimationType(ANIMATION_TYPE type)
{
   type_ = type;
}

Animatable::ANIMATION_TYPE
Animatable::GetAnimationType() const
{
   return type_;
}

void
Animatable::UpdateAnimationPoint()
{
   if (animatableCurrentState_.isReverse_
       and animatableCurrentState_.currentAnimationPoint_ == animationPoints_.begin())
   {
      return;
   }
   if (not animatableCurrentState_.isReverse_
       and animatableCurrentState_.currentAnimationPoint_ == animationPoints_.end())
   {
      return;
   }

   animatableCurrentState_.isReverse_ ? --animatableCurrentState_.currentAnimationPoint_
                                      : ++animatableCurrentState_.currentAnimationPoint_;

   auto updateReverseAnimation = [this]() {
      animatableCurrentState_.currentAnimationBegin_ =
         animatableCurrentState_.currentAnimationPoint_->m_end;

      if (animatableCurrentState_.currentAnimationPoint_ != animationPoints_.begin())
      {
         auto previousStep = std::prev(animatableCurrentState_.currentAnimationPoint_);
         animatableCurrentState_.currentAnimationEnd_ = previousStep->m_end;
      }
      else
      {
         animatableCurrentState_.currentAnimationEnd_ = animationStartPosition_;
      }
   };

   // If forward pass is finished
   if (animatableCurrentState_.currentAnimationPoint_ == animationPoints_.end())
   {
      // For reversable animations start going back
      if (type_ == Animatable::ANIMATION_TYPE::REVERSABLE)
      {
         animatableCurrentState_.isReverse_ = true;
         --animatableCurrentState_.currentAnimationPoint_;

         updateReverseAnimation();
      }
      // For loop animation position on the end
      else
      {
         ResetAnimation();
         animatableCurrentState_.animationFinished_ = true;
      }
   }
   else
   {
      if (animatableCurrentState_.isReverse_)
      {
         updateReverseAnimation();
      }
      else
      {
         animatableCurrentState_.currentAnimationBegin_ =
            animatableCurrentState_.currentAnimationEnd_;
         animatableCurrentState_.currentAnimationEnd_ =
            animatableCurrentState_.currentAnimationPoint_->m_end;
      }
   }
}

glm::vec2
Animatable::SetCorrectAnimationPoint(time::milliseconds& updateTime)
{
   auto animationValue = glm::vec2();

   ResetAnimation();

   auto animationDurationMs =
      time::Timer::ConvertToMs(animatableCurrentState_.currentAnimationPoint_->m_timeDuration);

   while (updateTime >= animationDurationMs)
   {
      updateTime -= animationDurationMs;
      animationValue += animatableCurrentState_.currentAnimationEnd_
                        - animatableCurrentState_.currentAnimationBegin_;

      UpdateAnimationPoint();
      animationDurationMs =
         time::Timer::ConvertToMs(animatableCurrentState_.currentAnimationPoint_->m_timeDuration);
   }

   return animationValue;
}

glm::vec2
Animatable::CalculateNextStep(time::milliseconds updateTime) const
{
   const auto startPosition = animatableCurrentState_.currentAnimationBegin_;
   const auto destination = animatableCurrentState_.currentAnimationEnd_;
   const auto animationDurationMs =
      time::Timer::ConvertToMs(animatableCurrentState_.currentAnimationPoint_->m_timeDuration);

   const auto timeLeft = static_cast< float >(
      (animationDurationMs - animatableCurrentState_.currentTimeElapsed_).count());

   // Make sure we don't divide by 0
   const auto sizeOfStep =
      (timeLeft > 0.0f) ? (static_cast< float >(updateTime.count()) / timeLeft) : 0.0f;

   const auto currentAnimationSectonLength = destination - startPosition;
   const auto currentAnimationStepSize =
      (currentAnimationSectonLength - animatableCurrentState_.currentAnimationDistance_)
      * sizeOfStep;

   return currentAnimationStepSize;
}

glm::vec2
Animatable::AnimateInCurrentSection(time::milliseconds updateTime)
{
   auto animationValue = CalculateNextStep(updateTime);

   // Object position after adding animation step
   animatableCurrentState_.currentAnimationPosition_ += animationValue;
   animatableCurrentState_.currentAnimationDistance_ += animationValue;
   animatableCurrentState_.currentTimeElapsed_ += updateTime;

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

   animatableCurrentState_.animationFinished_ = false;
   animatableCurrentState_.totalTimeElapsed_ += updateTime;

   auto currentAnimationStepSize = AnimateInCurrentSection(updateTime);
   if (animatableCurrentState_.currentTimeElapsed_
       < animatableCurrentState_.currentAnimationPoint_->m_timeDuration)
   {
      animateBy = currentAnimationStepSize;
   }
   else
   {
      animatableCurrentState_.currentTimeElapsed_ = time::milliseconds(0);
      animatableCurrentState_.currentAnimationDistance_ = glm::vec2(0.0f, 0.0f);

      if (animatableCurrentState_.isReverse_
          && animationPoints_.begin() == animatableCurrentState_.currentAnimationPoint_)
      {
         ResetAnimation();
         animatableCurrentState_.animationFinished_ = true;
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
   if (animatableCurrentState_.animationFinished_)
   {
      animatableCurrentState_.animationFinished_ = false;
      return glm::vec2{};
   }

   return Animate(updateTime);
}

bool
Animatable::AnimationFinished() const
{
   return animatableCurrentState_.animationFinished_;
}

AnimationPoint
Animatable::CreateAnimationNode(Object::ID parentID, const glm::vec2& position)
{
   const auto nodePosition =
      animationPoints_.empty() ? position : animationPoints_.back().m_end + glm::vec2(20.0f, 20.0f);
   auto newNode = AnimationPoint(parentID, nodePosition, time::seconds(2));
   AddAnimationNode(newNode);

   return newNode;
}

void
Animatable::AddAnimationNode(const AnimationPoint& newAnimationPoint)
{
   animationPoints_.push_back(newAnimationPoint);
}

void
Animatable::UpdateAnimationNode(const AnimationPoint& updatedAnimationPoint)
{
   auto updatedPointIt =
      std::find_if(animationPoints_.begin(), animationPoints_.end(),
                   [&updatedAnimationPoint](const auto animationPoint) {
                      return animationPoint.GetID() == updatedAnimationPoint.GetID();
                   });

   if (updatedPointIt != animationPoints_.end())
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
      std::find_if(animationPoints_.begin(), animationPoints_.end(),
                   [animationID](const auto& point) { return point.GetID() == animationID; });
   /*auto it = std::find(animationPoints_.begin(), animationPoints_.end(), pathNodeMapPosition);*/

   if (it != animationPoints_.end())
   {
      animationPoints_.erase(it);
   }
   else
   {
      Logger::Warn("Attempting to remove non existing node with ID={}", animationID);
   }
}

void
Animatable::SetAnimationKeypoints(std::vector< AnimationPoint >&& keypoints)
{
   animationPoints_ = std::move(keypoints);
   ResetAnimation();
}

std::vector< AnimationPoint >&
Animatable::GetAnimationKeypoints()
{
   return animationPoints_;
}

const std::vector< AnimationPoint >&
Animatable::GetAnimationKeypoints() const
{
   return animationPoints_;
}

time::seconds
Animatable::GetAnimationDuration() const
{
   auto totalDuration = time::seconds(0);

   for (const auto& animationPoint : animationPoints_)
   {
      totalDuration += animationPoint.m_timeDuration;
      totalDuration += animationPoint.m_pauseDuration;
   }

   // REVERSABLE animation takes twice as long
   return type_ == ANIMATION_TYPE::REVERSABLE ? 2 * totalDuration : totalDuration;
}

void
Animatable::UpdateAnimation(bool isReverse)
{
   if (isReverse)
   {
      animatableCurrentState_ = animatableStatesQueue_.GetLastState();
   }
   else
   {
      animatableStatesQueue_.PushState(animatableCurrentState_);
   }
}

void
Animatable::UpdateNodes()
{
}

void
Animatable::ResetAnimation()
{
   animatableCurrentState_.currentAnimationPoint_ = animationPoints_.begin();
   animatableCurrentState_.currentAnimationBegin_ = animationStartPosition_;
   animatableCurrentState_.currentAnimationEnd_ =
      animationPoints_.empty() ? animationStartPosition_
                               : animatableCurrentState_.currentAnimationPoint_->m_end;
   animatableCurrentState_.currentAnimationPosition_ = animationStartPosition_;
   animatableCurrentState_.currentAnimationDistance_ = glm::vec2(0.0f, 0.0f);
   animatableCurrentState_.isReverse_ = false;
   animatableCurrentState_.animationFinished_ = false;
   animatableCurrentState_.currentTimeElapsed_ = time::milliseconds(0);
   animatableCurrentState_.totalTimeElapsed_ = time::milliseconds(0);
}

time::milliseconds
Animatable::GetTotalTimeElapsed() const
{
   return animatableCurrentState_.totalTimeElapsed_;
}

void
Animatable::UpdateAnimationData()
{
   if (animatableCurrentState_.isReverse_)
   {
      if (animatableCurrentState_.currentAnimationPoint_ != animationPoints_.begin())
      {
         animatableCurrentState_.currentAnimationEnd_ =
            std::prev(animatableCurrentState_.currentAnimationPoint_)->m_end;
      }
      else
      {
         animatableCurrentState_.currentAnimationEnd_ = animationStartPosition_;
      }

      animatableCurrentState_.currentAnimationBegin_ =
         animatableCurrentState_.currentAnimationPoint_->m_end;
   }
   else
   {
      if (animatableCurrentState_.currentAnimationPoint_ != animationPoints_.begin())
      {
         animatableCurrentState_.currentAnimationBegin_ =
            std::prev(animatableCurrentState_.currentAnimationPoint_)->m_end;
      }
      else
      {
         animatableCurrentState_.currentAnimationBegin_ = animationStartPosition_;
      }

      animatableCurrentState_.currentAnimationEnd_ =
         animationPoints_.empty() ? animationStartPosition_
                                  : animatableCurrentState_.currentAnimationPoint_->m_end;
   }
}

void
Animatable::RenderAnimationSteps(bool choice)
{
   renderAnimationSteps_ = choice;
}

bool
Animatable::GetRenderAnimationSteps() const
{
   return renderAnimationSteps_;
}

void
Animatable::LockAnimationSteps(bool lock)
{
   lockAnimationSteps_ = lock;
}

bool
Animatable::GetLockAnimationSteps() const
{
   return lockAnimationSteps_;
}

void
Animatable::SetAnimationStartLocation(const glm::vec2& position)
{
   animationStartPosition_ = position;

   if (animatableCurrentState_.currentAnimationPoint_ == animationPoints_.begin())
   {
      animatableCurrentState_.currentAnimationBegin_ = animationStartPosition_;
      animatableCurrentState_.currentAnimationPosition_ = animationStartPosition_;
      animatableCurrentState_.currentAnimationDistance_ = glm::vec2(0.0f, 0.0f);
      animatableCurrentState_.currentTimeElapsed_ = time::milliseconds(0);
   }
}

glm::vec2
Animatable::GetAnimationStartLocation() const
{
   return animationStartPosition_;
}

} // namespace looper
