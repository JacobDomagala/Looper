#include <Enemy.hpp>
#include <Game.hpp>
#include <Level.hpp>
#include <Timer.hpp>
#include <Weapon.hpp>

Enemy::Enemy(Game& game, const glm::vec2& pos, const glm::ivec2& size, const std::string& sprite) : GameObject(game, pos, size, sprite)
{
   m_maxHP = 100;
   m_currentState.m_currentHP = m_maxHP;
   m_currentState.m_visionRange = 1000.0f;
   m_weapon = std::make_unique< Glock >();
   m_currentState.m_combatStarted = false;

   m_timer.ToggleTimer();
   m_currentState.m_initialPosition = GameObject::m_currentState.m_centeredLocalPosition;
   m_currentState.m_currentNodeIdx =
      m_gameHandle.GetLevel().GetPathfinder().FindNodeIdx(GameObject::m_currentState.m_centeredLocalPosition);
}

void
Enemy::DealWithPlayer()
{
   // Game::GetInstance( ).RenderText( std::to_string( m_currentNodeIdx ), glm::vec2( 125.5f, 125.5f ), 1.0f, glm::vec3( 1.0f, 0.0f, 0.0f )
   // );

   auto collided = m_gameHandle.IsPlayerInVision(this, static_cast< int32_t >(m_currentState.m_visionRange));

   m_timer.ToggleTimer();

   // player in enemy's sight of vision
   if (collided)
   {
      m_currentState.m_action = ACTION::SHOOTING;
      m_currentState.m_lastPlayersPos = m_gameHandle.GetPlayer().GetCenteredLocalPosition();
      m_currentState.m_timeSinceCombatEnded = 0.0f;
      m_currentState.m_CurrentAnimationIndex = 0;
      m_currentState.m_counter = glm::vec2(0.0f, 0.0f);
      m_currentState.m_reverse = false;

      if (m_currentState.m_combatStarted)
      {
         m_currentState.m_timeSinceCombatStarted += m_timer.GetDeltaTime();

         if (m_currentState.m_timeSinceCombatStarted > m_currentState.m_reactionTime)
         {
            Shoot();
            SetTargetShootPosition(m_gameHandle.GetPlayer().GetCenteredLocalPosition());
         }
      }
      else
      {
         SetTargetShootPosition(m_gameHandle.GetPlayer().GetCenteredLocalPosition());
      }

      m_timer.ResetAccumulator();
   }
   // player is out of range, clear enemy's 'memory'
   else
   {
      m_currentState.m_timeSinceCombatEnded += m_timer.GetDeltaTime();

      if ((m_currentState.m_action != ACTION::IDLE) && (m_currentState.m_timeSinceCombatEnded < 3.0f))
      {
         m_currentState.m_action = ACTION::CHASING_PLAYER;
         m_currentState.m_isChasingPlayer = true;
         ChasePlayer();
      }
      else
      {
         m_currentState.m_isChasingPlayer = false;
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

   SetCenteredLocalPosition(m_gameHandle.GetLevel().GetLocalVec(GameObject::m_currentState.m_centeredGlobalPosition));
   SetLocalPosition(m_gameHandle.GetLevel().GetLocalVec(GameObject::m_currentState.m_globalPosition));
}

void Enemy::Hit(int32_t /*dmg*/)
{
   // currentHP -= dmg;
   SetColor({1.0f, 0.0f, 0.0f});
}

bool
Enemy::Visible() const
{
   return (m_currentState.m_currentHP > 0);
}

void
Enemy::SetTargetShootPosition(const glm::vec2& playerPos)
{
   auto& playerSize = m_gameHandle.GetPlayer().GetSize();

   // compute small offset value which simulates the 'aim wiggle'
   auto xOffset = fmod(rand(), playerSize.x) + (-playerSize.x / 2);
   auto yOffset = fmod(rand(), playerSize.y) + (-playerSize.y / 2);

   m_currentState.m_targetShootPosition = (playerPos + glm::vec2(xOffset, yOffset));
   m_currentState.m_combatStarted = true;
}

void
Enemy::Shoot()
{
   m_currentState.m_timeSinceLastShot += m_timer.GetDeltaTime();

   m_gameHandle.RenderText("POW POW", glm::vec2(128.0f, 64.0f), 1.0f, glm::vec3(0.0f, 0.1f, 0.4f));
   if (glm::length(static_cast< glm::vec2 >(m_gameHandle.GetPlayer().GetCenteredLocalPosition()
                                            - GameObject::m_currentState.m_centeredLocalPosition))
       <= m_weapon->GetRange())
   {
      if (m_currentState.m_timeSinceLastShot >= m_weapon->GetReloadTime())
      {
         auto collided = m_gameHandle.CheckBulletCollision(this, m_gameHandle.GetLevel().GetGlobalVec(m_currentState.m_targetShootPosition),
                                                           m_weapon->GetRange());

         // if we hit anything draw a line
         if (collided.first != glm::ivec2(0, 0))
         {
            m_gameHandle.DrawLine(GameObject::m_currentState.m_centeredGlobalPosition,
                                  m_gameHandle.GetLevel().GetGlobalVec(collided.first));
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
   auto playerPos = m_currentState.m_lastPlayersPos; // m_gameHandle.GetPlayer().GetCenteredLocalPosition();
   auto moveBy = 500.0f * m_gameHandle.GetDeltaTime();

   auto distanceToNode =
      glm::length(static_cast< glm::vec2 >(m_currentState.m_targetMovePosition - GameObject::m_currentState.m_centeredLocalPosition));

   if ((distanceToNode < 5.0f) && (distanceToNode != 0.0f))
   {
      Move(m_currentState.m_targetMovePosition - GameObject::m_currentState.m_centeredLocalPosition, false);
      m_currentState.m_currentNodeIdx =
         m_gameHandle.GetLevel().GetPathfinder().FindNodeIdx(GameObject::m_currentState.m_centeredLocalPosition);
   }
   else if (m_currentState.m_targetMovePosition
            == GameObject::m_currentState.m_centeredLocalPosition /*|| m_targetMovePosition == glm::ivec2(0, 0)*/)
   {
      m_currentState.m_currentNodeIdx =
         m_gameHandle.GetLevel().GetPathfinder().FindNodeIdx(GameObject::m_currentState.m_centeredLocalPosition);
      m_currentState.m_targetMovePosition =
         m_gameHandle.GetLevel().GetPathfinder().GetNearestPosition(m_currentState.m_currentNodeIdx, playerPos);
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
            m_gameHandle.GetLevel().GetPathfinder().GetNearestNode(GameObject::m_currentState.m_centeredLocalPosition);
      }

      m_currentState.m_targetMovePosition =
         m_gameHandle.GetLevel().GetPathfinder().GetNearestPosition(m_currentState.m_currentNodeIdx, playerPos);

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
   m_gameHandle.RenderText("RETURNING", glm::vec2(128.0f, 256.0f), 1.0f, glm::vec3(0.0f, 0.1f, 0.4f));

   auto moveBy = m_currentState.m_movementSpeed * m_gameHandle.GetDeltaTime();
   auto vectorToInitialPos =
      static_cast< glm::vec2 >(m_currentState.m_initialPosition - GameObject::m_currentState.m_centeredLocalPosition);
   auto lengthToInitialPos = glm::length(vectorToInitialPos);
   auto distanceToNode =
      glm::length(static_cast< glm::vec2 >(m_currentState.m_targetMovePosition - GameObject::m_currentState.m_centeredLocalPosition));

   if (lengthToInitialPos < 5.0f)
   {
      Move(vectorToInitialPos, false);
      m_currentState.m_currentNodeIdx =
         m_gameHandle.GetLevel().GetPathfinder().FindNodeIdx(GameObject::m_currentState.m_centeredLocalPosition);
      m_currentState.m_isAtInitialPos = true;
      return;
   }
   else if ((distanceToNode < 5.0f) && (distanceToNode != 0.0f))
   {
      Move(m_currentState.m_targetMovePosition - GameObject::m_currentState.m_centeredLocalPosition, false);
      m_currentState.m_currentNodeIdx =
         m_gameHandle.GetLevel().GetPathfinder().FindNodeIdx(GameObject::m_currentState.m_centeredLocalPosition);
   }
   else if (m_currentState.m_targetMovePosition == GameObject::m_currentState.m_centeredLocalPosition)
   {
      m_currentState.m_currentNodeIdx =
         m_gameHandle.GetLevel().GetPathfinder().FindNodeIdx(GameObject::m_currentState.m_centeredLocalPosition);
      m_currentState.m_targetMovePosition = m_gameHandle.GetLevel().GetPathfinder().GetNearestPosition(
         /*m_centeredLocalPosition*/ m_currentState.m_currentNodeIdx, m_currentState.m_initialPosition);
      auto move = m_currentState.m_targetMovePosition - GameObject::m_currentState.m_centeredLocalPosition;
      auto nMove = glm::normalize(static_cast< glm::vec2 >(move));

      Move(nMove * moveBy, false);
   }
   else
   {
      m_currentState.m_targetMovePosition = m_gameHandle.GetLevel().GetPathfinder().GetNearestPosition(
         /*m_centeredLocalPosition*/ m_currentState.m_currentNodeIdx, m_currentState.m_initialPosition);

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
      m_statesQueue.push_back(m_currentState);
      if (m_statesQueue.size() >= NUM_FRAMES_TO_SAVE)
      {
         m_statesQueue.pop_front();
      }
   }
}

void
Enemy::Render(Window& window, const Shaders& program)
{
   if (!m_currentState.m_combatStarted && m_currentState.m_isAtInitialPos)
   {
      Animate();
   }

   GameObject::Render(window, program);
}

void
Enemy::Animate()
{
   auto deltaTime = m_gameHandle.GetDeltaTime();
   auto currentAnimation = m_positions.at(m_currentState.m_CurrentAnimationIndex);
   auto currentAnimationStep = glm::vec2(currentAnimation.x * deltaTime, currentAnimation.y * deltaTime);

   if (!m_currentState.m_reverse)
   {
      auto nextStep = m_currentState.m_counter + currentAnimationStep;

      if ((nextStep.x <= currentAnimation.x) && (nextStep.y <= currentAnimation.y))
      {
         m_currentState.m_counter += currentAnimationStep;
      }
      else if ((nextStep.x >= currentAnimation.x) && (nextStep.y >= currentAnimation.y))
      {
         m_currentState.m_counter = currentAnimation;

         Move(m_currentState.m_counter, false);

         ++m_currentState.m_CurrentAnimationIndex;
         if (m_currentState.m_CurrentAnimationIndex == m_positions.size())
         {
            m_currentState.m_reverse = true;
            --m_currentState.m_CurrentAnimationIndex;
         }

         m_currentState.m_counter = glm::vec2();
      }
   }
   else // reverse
   {
      currentAnimation *= -1.0f;
      currentAnimationStep *= -1.0f;

      auto nextStep = m_currentState.m_counter + currentAnimationStep;

      if ((nextStep.x >= currentAnimation.x) && (nextStep.y >= currentAnimation.y))
      {
         m_currentState.m_counter += currentAnimationStep;
      }
      else if ((nextStep.x <= currentAnimation.x) && (nextStep.y <= currentAnimation.y))
      {
         m_currentState.m_counter = currentAnimation;

         Move(m_currentState.m_counter, false);

         --m_currentState.m_CurrentAnimationIndex;
         if (m_currentState.m_CurrentAnimationIndex == -1)
         {
            m_currentState.m_reverse = false;
            ++m_currentState.m_CurrentAnimationIndex;
         }

         m_currentState.m_counter = glm::vec2();
      }
   }

   Move(m_currentState.m_counter, false);
}
