#include <Application.hpp>
#include <Enemy.hpp>
#include <Game.hpp>
#include <Level.hpp>
#include <Timer.hpp>
#include <Weapon.hpp>

namespace dgame {

Enemy::Enemy(Application& context, const glm::vec2& pos, const glm::ivec2& size,
             const std::string& sprite, const std::vector< AnimationPoint >& keypoints,
             Animatable::ANIMATION_TYPE animationType)
   : GameObject(context, pos, size, sprite, TYPE::ENEMY), Animatable(animationType)
{
   m_currentState.m_currentHP = m_maxHP;
   m_currentState.m_visionRange = 1000.0f;
   m_weapon = std::make_unique< Glock >();
   m_currentState.m_combatStarted = false;
   m_animationPoints = keypoints;

   m_timer.ToggleTimer();
   m_initialPosition = GameObject::m_currentState.m_localPosition;

   m_animationStartPosition = m_initialPosition;
   ResetAnimation();
}

void
Enemy::DealWithPlayer()
{
   auto gameHandle = ConvertToGameHandle();

   const auto playerPosition = gameHandle->GetPlayer()->GetCenteredGlobalPosition();
   const auto playerInVision = gameHandle->GetLevel().CheckCollisionAlongTheLine(
      GameObject::m_currentState.m_centeredGlobalPosition, playerPosition);

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

void Enemy::Hit(int32_t /*dmg*/)
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
Enemy::SetTargetShootPosition(const glm::vec2& playerPos)
{
   auto playerSize = m_appHandle.GetPlayer()->GetSize();

   // compute small offset value which simulates the 'aim wiggle'
   auto xOffset = fmod(rand(), playerSize.x) + (-playerSize.x / 2);
   auto yOffset = fmod(rand(), playerSize.y) + (-playerSize.y / 2);

   m_currentState.m_targetShootPosition = (playerPos + glm::vec2(xOffset, yOffset));
   m_currentState.m_combatStarted = true;
}

void
Enemy::Shoot()
{
   m_currentState.m_timeSinceLastShot += m_timer.GetFloatDeltaTime();

   if (glm::length(static_cast< glm::vec2 >(m_appHandle.GetPlayer()->GetCenteredGlobalPosition()
                                            - GameObject::m_currentState.m_centeredGlobalPosition))
       <= static_cast< float >(m_weapon->GetRange()))
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

   const auto curPosition = GameObject::m_currentState.m_centeredGlobalPosition;
   const auto tiles = pathFinder.GetPath(curPosition, targetPosition);

   if (tiles.size() > 0)
   {
      const auto moveVal =
         moveBy * glm::normalize(pathFinder.GetNodeFromID(tiles.back()).m_position - curPosition);
      Move(moveVal, false);
   }
   else if (exactPosition)
   {
      const auto moveVal = moveBy * glm::normalize(targetPosition - curPosition);
      Move(moveVal, false);

      constexpr auto errorTreshold = 3.0f;
      const auto distanceToDest =
         targetPosition - GameObject::m_currentState.m_centeredGlobalPosition;

      // If Enemy is really close to target destination, just put it there
      if (glm::length(distanceToDest) < errorTreshold)
      {
         Move(distanceToDest, false);
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
         Move(Animate(m_appHandle.GetDeltaTime()), false);
      }

      m_statesQueue.push_back(m_currentState);
      if (m_statesQueue.size() >= NUM_FRAMES_TO_SAVE)
      {
         m_statesQueue.pop_front();
      }
   }

   Animatable::Update(isReverse);
}

} // namespace dgame
