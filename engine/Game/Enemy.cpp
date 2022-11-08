#include <Application.hpp>
#include <Enemy.hpp>
#include <Game.hpp>
#include <Level.hpp>
#include <Timer.hpp>
#include <Weapon.hpp>

#include <random>

namespace looper {

Enemy::Enemy(Application& context, const glm::vec2& pos, const glm::ivec2& size,
             const std::string& textureName, const std::vector< AnimationPoint >& keypoints,
             Animatable::ANIMATION_TYPE animationType)
   : GameObject(context, pos, size, textureName, TYPE::ENEMY),
     Animatable(animationType),
     m_weapon(std::make_unique< Glock >()),
     m_initialPosition(m_currentGameObjectState.m_position)
{
   m_currentState.m_currentHP = m_maxHP;
   m_currentState.m_visionRange = 1000.0f;

   m_currentState.m_combatStarted = false;
   m_animationPoints = keypoints;

   m_timer.ToggleTimer();

   m_animationStartPosition = m_initialPosition;
   ResetAnimation();
}

void
Enemy::DealWithPlayer()
{
   auto* gameHandle = ConvertToGameHandle();

   const auto playerPosition = gameHandle->GetPlayer()->GetCenteredPosition();
   const auto playerInVision = gameHandle->GetLevel().CheckCollisionAlongTheLine(
      m_currentGameObjectState.m_centeredPosition, playerPosition);

   m_timer.ToggleTimer();

   // player in enemy's sight of vision
   if (playerInVision)
   {
      m_currentState.m_action = ACTION::SHOOTING;
      m_currentState.m_lastPlayersPos = playerPosition;
      m_currentState.m_timeSinceCombatEnded = Timer::milliseconds(0);
      ResetAnimation();

      if (m_currentState.m_combatStarted)
      {
         m_currentState.m_timeSinceCombatStarted += m_timer.GetFloatDeltaTime();

         if (m_currentState.m_timeSinceCombatStarted > m_currentState.m_reactionTime)
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
      m_currentState.m_timeSinceCombatEnded += m_timer.GetMsDeltaTime();
      const auto chaseTime = Timer::seconds(3);

      if ((m_currentState.m_action != ACTION::IDLE)
          && (m_currentState.m_timeSinceCombatEnded < chaseTime))
      {
         m_currentState.m_action = ACTION::CHASING_PLAYER;
         ChasePlayer();
      }
      else
      {
         if (!m_currentState.m_isAtInitialPos)
         {
            m_currentState.m_action = ACTION::RETURNING;
            ReturnToInitialPosition();
         }
         else
         {
            m_currentState.m_action = ACTION::IDLE;
            ClearPositions();
         }
      }
   }
}

void
Enemy::Hit(int32_t /*dmg*/)
{
   // currentHP -= dmg;
   SetColor({1.0f, 0.0f, 0.0f});
}

std::string
Enemy::GetWeapon() const
{
   return m_weapon->GetName();
}

int32_t
Enemy::GetDmg() const
{
   return m_weapon->GetDamage();
}

glm::ivec2
Enemy::GetInitialPosition() const
{
   return m_initialPosition;
}

bool
Enemy::Visible() const
{
   return (m_currentState.m_currentHP > 0);
}

void
Enemy::SetTargetShootPosition(const glm::vec2& targetPosition)
{
   auto playerSize = m_appHandle.GetPlayer()->GetSize();

   std::random_device rd;
   std::mt19937 mt(rd());
   std::uniform_real_distribution< double > dist(1.0, 10.0);

   // compute small offset value which simulates the 'aim wiggle'
   auto xOffset = fmod(dist(mt), playerSize.x) + (-playerSize.x / 2.0);
   auto yOffset = fmod(dist(mt), playerSize.y) + (-playerSize.y / 2.0);

   m_currentState.m_targetShootPosition = (targetPosition + glm::vec2(xOffset, yOffset));
   m_currentState.m_combatStarted = true;
}

void
Enemy::Shoot()
{
   m_currentState.m_timeSinceLastShot += m_timer.GetFloatDeltaTime();

   if (glm::length(m_appHandle.GetPlayer()->GetCenteredPosition()
                   - m_currentGameObjectState.m_centeredPosition)
       <= (m_weapon->GetRange()))
   {
      if (m_currentState.m_timeSinceLastShot >= m_weapon->GetReloadTime())
      {
         // Shoot
         m_currentState.m_timeSinceLastShot = 0.0f;
      }
   }
   else
   {
      ChasePlayer();
   }
}

bool
Enemy::MoveToPosition(const glm::vec2& targetPosition, bool exactPosition)
{
   bool destinationReached = false;

   auto* gameHandle = ConvertToGameHandle();

   auto moveBy =
      m_currentState.m_movementSpeed * static_cast< float >(gameHandle->GetDeltaTime().count());

   auto& pathFinder = gameHandle->GetLevel().GetPathfinder();

   const auto curPosition = m_currentGameObjectState.m_centeredPosition;
   const auto tiles = pathFinder.GetPath(curPosition, targetPosition);

   if (!tiles.empty())
   {
      const auto moveVal =
         moveBy * glm::normalize(pathFinder.GetNodeFromID(tiles.back()).m_position - curPosition);
      Move(moveVal);
   }
   else if (exactPosition)
   {
      const auto moveVal = moveBy * glm::normalize(targetPosition - curPosition);
      Move(moveVal);

      constexpr auto errorTreshold = 3.0f;
      const auto distanceToDest = targetPosition - m_currentGameObjectState.m_centeredPosition;

      // If Enemy is really close to target destination, just put it there
      if (glm::length(distanceToDest) < errorTreshold)
      {
         Move(distanceToDest);
         destinationReached = true;
      }
   }

   return destinationReached;
}

void
Enemy::ChasePlayer()
{
   MoveToPosition(m_currentState.m_lastPlayersPos);

   m_currentState.m_isAtInitialPos = false;
}

void
Enemy::ReturnToInitialPosition()
{
   m_currentState.m_isAtInitialPos = MoveToPosition(m_initialPosition, true);
}

void
Enemy::ClearPositions()
{
   m_currentState.m_targetShootPosition = glm::vec2(0.0f, 0.0f);
   m_currentState.m_combatStarted = false;
   m_currentState.m_timeSinceCombatStarted = 0.0f;
   m_currentState.m_timeSinceLastShot = 0.0f;
}

void
Enemy::UpdateInternal(bool isReverse)
{
   if (isReverse)
   {
      m_currentState = m_statesQueue.back();
      m_statesQueue.pop_back();
   }
   else
   {
      if (!m_currentState.m_combatStarted && m_currentState.m_isAtInitialPos)
      {
         Move(Animate(m_appHandle.GetDeltaTime()));
      }

      m_statesQueue.push_back(m_currentState);
      if (m_statesQueue.size() >= NUM_FRAMES_TO_SAVE)
      {
         m_statesQueue.pop_front();
      }
   }

   Animatable::Update(isReverse);
}

} // namespace looper
