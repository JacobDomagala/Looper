#include <application.hpp>
#include <enemy.hpp>
#include <game.hpp>
#include <level.hpp>
#include <utils/time/timer.hpp>
#include <weapon.hpp>

#include <random>

namespace looper {

Enemy::Enemy(Application* context, const glm::vec3& pos, const glm::ivec2& size,
             const std::string& textureName, const std::vector< AnimationPoint >& keypoints,
             Animatable::ANIMATION_TYPE animationType)
   : GameObject(context, pos, size, textureName, ObjectType::ENEMY),
     Animatable(animationType),
     initialPosition_(currentGameObjectState_.position_)
{
   currentState_.currentHP_ = maxHP_;
   currentState_.visionRange_ = 1000.0f;

   currentState_.combatStarted_ = false;
   m_animationPoints = keypoints;

   timer_.ToggleTimer();

   m_animationStartPosition = initialPosition_;
   ResetAnimation();
}

Enemy::Enemy(Application* context, const glm::vec2& pos, const glm::ivec2& size,
             const std::string& textureName, const std::vector< AnimationPoint >& keypoints,
             Animatable::ANIMATION_TYPE animationType)
   : Enemy(context, glm::vec3{pos, 0.0f}, size, textureName, keypoints, animationType)
{
}

void
Enemy::Setup(Application* context, const glm::vec2& pos, const glm::ivec2& size,
             const std::string& textureName, const std::vector< AnimationPoint >& keypoints,
             Animatable::ANIMATION_TYPE animationType)
{
   GameObject::Setup(context, glm::vec3{pos, 0.0f}, size, textureName, ObjectType::ENEMY);
   SetAnimationType(animationType);

   currentState_.currentHP_ = maxHP_;
   currentState_.visionRange_ = 1000.0f;

   currentState_.combatStarted_ = false;
   m_animationPoints = keypoints;

   timer_.ToggleTimer();

   m_animationStartPosition = initialPosition_;
   ResetAnimation();
}

void
Enemy::DealWithPlayer()
{
   auto* gameHandle = ConvertToGameHandle();

   const auto playerPosition = gameHandle->GetPlayer().GetCenteredPosition();
   const auto playerInVision = gameHandle->GetLevel().CheckCollisionAlongTheLine(
      currentGameObjectState_.centeredPosition_, playerPosition);

   timer_.ToggleTimer();

   // player in enemy's sight of vision
   if (playerInVision)
   {
      currentState_.action_ = ACTION::SHOOTING;
      currentState_.lastPlayersPos_ = playerPosition;
      currentState_.timeSinceCombatEnded_ = time::milliseconds(0);
      ResetAnimation();

      if (currentState_.combatStarted_)
      {
         currentState_.timeSinceCombatStarted_ += timer_.GetFloatDeltaTime();

         if (currentState_.timeSinceCombatStarted_ > currentState_.reactionTime_)
         {
            Shoot();
            SetTargetShootPosition(playerPosition);
         }
      }
      else
      {
         SetTargetShootPosition(playerPosition);
      }
   }
   // player is out of range, clear enemy's 'memory'
   else
   {
      currentState_.timeSinceCombatEnded_ += timer_.GetMsDeltaTime();
      const auto chaseTime = time::seconds(3);

      if ((currentState_.action_ != ACTION::IDLE)
          && (currentState_.timeSinceCombatEnded_ < chaseTime))
      {
         currentState_.action_ = ACTION::CHASING_PLAYER;
         ChasePlayer();
      }
      else
      {
         if (!currentState_.isAtInitialPos_)
         {
            currentState_.action_ = ACTION::RETURNING;
            ReturnToInitialPosition();
         }
         else
         {
            currentState_.action_ = ACTION::IDLE;
            ClearPositions();
         }
      }
   }
}

void
Enemy::Hit(int32_t /*dmg*/)
{
   // currentHP -= dmg;
   SetColor({1.0f, 0.0f, 0.0, 0.75f});
}

glm::ivec2
Enemy::GetInitialPosition() const
{
   return initialPosition_;
}

bool
Enemy::Visible() const
{
   return (currentState_.currentHP_ > 0);
}

void
Enemy::SetTargetShootPosition(const glm::vec2& targetPosition)
{
   auto playerSize = appHandle_->GetPlayer().GetSize();

   std::random_device rd;
   std::mt19937 mt(rd());
   std::uniform_real_distribution< double > dist(1.0, 10.0);

   // compute small offset value which simulates the 'aim wiggle'
   auto xOffset = fmod(dist(mt), playerSize.x) + (-playerSize.x / 2.0);
   auto yOffset = fmod(dist(mt), playerSize.y) + (-playerSize.y / 2.0);

   currentState_.targetShootPosition_ = (targetPosition + glm::vec2(xOffset, yOffset));
   currentState_.combatStarted_ = true;
}

void
Enemy::Shoot()
{
   currentState_.timeSinceLastShot_ += timer_.GetFloatDeltaTime();

   if (glm::length(appHandle_->GetPlayer().GetCenteredPosition()
                   - currentGameObjectState_.centeredPosition_)
       <= (300.0f))
   {
      if (currentState_.timeSinceLastShot_ >= 0.3f)
      {
         // Shoot
         currentState_.timeSinceLastShot_ = 0.0f;
      }
   }
   else
   {
      ChasePlayer();
   }
}

void
Enemy::EnemyMove(const glm::vec2& moveBy)
{
   if (appHandle_->IsGame())
   {
      auto prevPosition = initialPosition_;

      if (GameObject::gameObjectStatesQueue_.GetNumFrames() > 1)
      {
         prevPosition = GetPreviousPosition();
      }

      const auto direction = currentGameObjectState_.position_ - prevPosition;

      currentState_.viewAngle_ = glm::atan(direction.y, direction.x);

      sprite_.Rotate(currentState_.viewAngle_);
   }

   Move(moveBy);
}

bool
Enemy::MoveToPosition(const glm::vec2& targetPosition, bool exactPosition)
{
   bool destinationReached = false;

   auto* gameHandle = ConvertToGameHandle();

   auto moveBy =
      currentState_.movementSpeed_ * static_cast< float >(gameHandle->GetDeltaTime().count());

   auto& pathFinder = gameHandle->GetLevel().GetPathfinder();

   const auto curPosition = glm::vec2(currentGameObjectState_.centeredPosition_);
   const auto tiles = pathFinder.GetPath(curPosition, targetPosition);

   if (!tiles.empty())
   {
      const auto moveVal =
         moveBy * glm::normalize(pathFinder.GetNodeFromID(tiles.back()).position_ - curPosition);
      EnemyMove(moveVal);
   }
   else if (exactPosition)
   {
      const auto moveVal = moveBy * glm::normalize(targetPosition - curPosition);
      EnemyMove(moveVal);

      constexpr auto errorTreshold = 3.0f;
      const auto distanceToDest =
         targetPosition - glm::vec2(currentGameObjectState_.centeredPosition_);

      // If Enemy is really close to target destination, just put it there
      if (glm::length(distanceToDest) < errorTreshold)
      {
         EnemyMove(distanceToDest);
         destinationReached = true;
      }
   }

   return destinationReached;
}

void
Enemy::ChasePlayer()
{
   MoveToPosition(currentState_.lastPlayersPos_);

   currentState_.isAtInitialPos_ = false;
}

void
Enemy::ReturnToInitialPosition()
{
   currentState_.isAtInitialPos_ = MoveToPosition(initialPosition_, true);
}

void
Enemy::ClearPositions()
{
   currentState_.targetShootPosition_ = glm::vec2(0.0f, 0.0f);
   currentState_.combatStarted_ = false;
   currentState_.timeSinceCombatStarted_ = 0.0f;
   currentState_.timeSinceLastShot_ = 0.0f;
}

void
Enemy::UpdateInternal(bool isReverse)
{
   if (isReverse)
   {
      currentState_ = statesQueue_.GetLastState();
   }
   else
   {
      DealWithPlayer();

      if (!currentState_.combatStarted_ && currentState_.isAtInitialPos_)
      {
         EnemyMove(Animate(appHandle_->GetDeltaTime()));
      }

      statesQueue_.PushState(currentState_);
   }

   Animatable::UpdateAnimation(isReverse);
}

} // namespace looper
