#include <Application.hpp>
#include <Enemy.hpp>
#include <Game.hpp>
#include <Level.hpp>
#include <Timer.hpp>
#include <Weapon.hpp>

namespace dgame {

Enemy::Enemy(Application& context, const glm::vec2& pos, const glm::ivec2& size, const std::string& sprite,
             const std::vector< AnimationPoint >& keypoints, Animatable::ANIMATION_TYPE animationType)
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
   auto collided = gameHandle->IsPlayerInVision(this, static_cast< int32_t >(m_currentState.m_visionRange));
   const auto playerPosition = gameHandle->GetPlayer()->GetCenteredLocalPosition();

   m_timer.ToggleTimer();

   // player in enemy's sight of vision
   if (collided)
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

      if ((m_currentState.m_action != ACTION::IDLE) && (m_currentState.m_timeSinceCombatEnded < chaseTime))
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

   // SetCenteredLocalPosition(gameHandle->GetLevel().GetLocalVec(GameObject::m_currentState.m_centeredLocalPosition));
   // SetLocalPosition(gameHandle->GetLevel().GetLocalVec(GameObject::m_currentState.m_localPosition));
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
   auto gameHandle = ConvertToGameHandle();

   m_currentState.m_timeSinceLastShot += m_timer.GetFloatDeltaTime();

   m_appHandle.RenderText("POW POW", glm::vec2(128.0f, 64.0f), 1.0f, glm::vec3(0.0f, 0.1f, 0.4f));
   if (glm::length(static_cast< glm::vec2 >(m_appHandle.GetPlayer()->GetCenteredLocalPosition()
                                            - GameObject::m_currentState.m_centeredLocalPosition))
       <= static_cast<float>(m_weapon->GetRange()))
   {
      if (m_currentState.m_timeSinceLastShot >= m_weapon->GetReloadTime())
      {
         auto collided = gameHandle->CheckBulletCollision(this, m_appHandle.GetLevel().GetGlobalVec(m_currentState.m_targetShootPosition),
                                                          m_weapon->GetRange());

         // if we hit anything draw a line
         if (collided.first != glm::ivec2(0, 0))
         {
            // gameHandle->DrawLine(GameObject::m_currentState.m_centeredGlobalPosition,
            // m_appHandle.GetLevel().GetGlobalVec(collided.first));
         }

         m_currentState.m_timeSinceLastShot = 0.0f;
      }
   }
   else
   {
      ChasePlayer();
   }
}

void
Enemy::ChasePlayer()
{
   auto* gameHandle = ConvertToGameHandle();

   const auto& playerPos = m_currentState.m_lastPlayersPos; // m_contextHandle.GetPlayer()->GetCenteredLocalPosition();
   const auto moveBy = m_currentState.m_movementSpeed * static_cast<float>(gameHandle->GetDeltaTime().count());

   const auto distanceToNode = glm::length(static_cast< glm::vec2 >(playerPos - GameObject::m_currentState.m_centeredLocalPosition));

   if ((distanceToNode < 5.0f) && (distanceToNode != 0.0f))
   {
      Move(m_currentState.m_targetMovePosition - GameObject::m_currentState.m_centeredLocalPosition, false);
      m_currentState.m_currentNodeIdx =
         gameHandle->GetLevel().GetPathfinder().FindNodeIdx(GameObject::m_currentState.m_centeredLocalPosition);
   }
   else if (m_currentState.m_targetMovePosition
            == GameObject::m_currentState.m_centeredLocalPosition /*|| m_targetMovePosition == glm::ivec2(0, 0)*/)
   {
      m_currentState.m_currentNodeIdx =
         gameHandle->GetLevel().GetPathfinder().FindNodeIdx(GameObject::m_currentState.m_centeredLocalPosition);
      m_currentState.m_targetMovePosition =
         gameHandle->GetLevel().GetPathfinder().GetNearestPosition(m_currentState.m_currentNodeIdx, playerPos);
      if (m_currentState.m_targetMovePosition != glm::ivec2(0, 0))
      {
         auto move = m_currentState.m_targetMovePosition - GameObject::m_currentState.m_centeredLocalPosition;
         auto nMove = glm::normalize(static_cast< glm::vec2 >(move));

         Move(nMove * moveBy, false);
      }
   }
   else
   {
      if (m_currentState.m_targetMovePosition == glm::ivec2())
      {
         m_currentState.m_currentNodeIdx =
            gameHandle->GetLevel().GetPathfinder().GetNearestNode(GameObject::m_currentState.m_centeredLocalPosition);
      }

      m_currentState.m_targetMovePosition = playerPos;
      // gameHandle->GetLevel().GetPathfinder().GetNearestPosition(m_currentState.m_currentNodeIdx, playerPos);

      if (m_currentState.m_targetMovePosition != glm::ivec2(0, 0))
      {
         auto move = m_currentState.m_targetMovePosition - GameObject::m_currentState.m_centeredLocalPosition;
         auto nMove = glm::normalize(static_cast< glm::vec2 >(move));

         Move(nMove * moveBy, false);
      }
   }

   m_currentState.m_isAtInitialPos = false;
}

void
Enemy::ReturnToInitialPosition()
{
   auto gameHandle = ConvertToGameHandle();

   auto moveBy = m_currentState.m_movementSpeed * static_cast<float>(gameHandle->GetDeltaTime().count());
   auto vectorToInitialPos = static_cast< glm::vec2 >(m_initialPosition - GameObject::m_currentState.m_centeredLocalPosition);
   auto lengthToInitialPos = glm::length(vectorToInitialPos);
   auto distanceToNode =
      glm::length(static_cast< glm::vec2 >(m_currentState.m_targetMovePosition - GameObject::m_currentState.m_centeredLocalPosition));

   if (lengthToInitialPos < 5.0f)
   {
      Move(vectorToInitialPos, false);
      m_currentState.m_currentNodeIdx =
         gameHandle->GetLevel().GetPathfinder().FindNodeIdx(GameObject::m_currentState.m_centeredLocalPosition);
      m_currentState.m_isAtInitialPos = true;
   }
   else if ((distanceToNode < 5.0f) && (distanceToNode != 0.0f))
   {
      Move(m_currentState.m_targetMovePosition - GameObject::m_currentState.m_centeredLocalPosition, false);
      m_currentState.m_currentNodeIdx =
         gameHandle->GetLevel().GetPathfinder().FindNodeIdx(GameObject::m_currentState.m_centeredLocalPosition);
   }
   else if (m_currentState.m_targetMovePosition == GameObject::m_currentState.m_centeredLocalPosition)
   {
      m_currentState.m_currentNodeIdx =
         gameHandle->GetLevel().GetPathfinder().FindNodeIdx(GameObject::m_currentState.m_centeredLocalPosition);
      m_currentState.m_targetMovePosition = gameHandle->GetLevel().GetPathfinder().GetNearestPosition(
         /*m_centeredLocalPosition*/ m_currentState.m_currentNodeIdx, m_initialPosition);
      auto move = m_currentState.m_targetMovePosition - GameObject::m_currentState.m_centeredLocalPosition;
      auto nMove = glm::normalize(static_cast< glm::vec2 >(move));

      Move(nMove * moveBy, false);
   }
   else
   {
      m_currentState.m_targetMovePosition = gameHandle->GetLevel().GetPathfinder().GetNearestPosition(
         /*m_centeredLocalPosition*/ m_currentState.m_currentNodeIdx, m_initialPosition);

      if (m_currentState.m_targetMovePosition != glm::ivec2(0, 0))
      {
         auto move = m_currentState.m_targetMovePosition - GameObject::m_currentState.m_centeredLocalPosition;
         auto nMove = glm::normalize(static_cast< glm::vec2 >(move));

         Move(nMove * moveBy, false);
      }
   }
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
