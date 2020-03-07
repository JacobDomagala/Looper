#pragma once

#include "Common.hpp"
#include "GameObject.hpp"
#include "Timer.hpp"
#include "Weapon.hpp"

#include <deque>
#include <vector>

class Window;

class Enemy : public GameObject
{
 public:
   Enemy(Game& game, const glm::vec2& pos, const glm::ivec2& size, const std::string& sprite);
   ~Enemy() override = default;

   bool
   Visible() const override;
   void
   Hit(int32_t dmg) override;
   void
   DealWithPlayer() override;

   void
   Render(Window& window, const Shaders& program) override;

   int32_t
   GetDmg() const
   {
      return m_weapon->GetDamage();
   }

   void
   Animate();

 private:
   void UpdateInternal(bool isReverse) override;

   enum class ACTION
   {
      IDLE,
      CHASING_PLAYER,
      SHOOTING,
      RETURNING
   };

   struct EnemyState
   {
      ACTION m_action = ACTION::IDLE;
      int32_t m_currentHP;
      glm::vec2 m_targetShootPosition;
      glm::ivec2 m_targetMovePosition;

      glm::ivec2 m_initialPosition;
      glm::ivec2 m_lastPlayersPos;

      bool m_isChasingPlayer = false;
      bool m_isAtInitialPos = true;

      float m_timeSinceCombatEnded = 0.0f;
      float m_timeSinceCombatStarted = 0.0f;
      float m_timeSinceLastShot = 0.0f;
      float m_reactionTime = 0.1f;
      float m_movementSpeed = 500.0f;
      float m_visionRange;

      uint8_t m_currentNodeIdx{};
      uint8_t m_destinationNodeIdx{};

      bool m_combatStarted;
      bool m_reverse = false;

      int32_t m_CurrentAnimationIndex = 0;
      glm::vec2 m_counter{0.0f, 0.0f};
   };

   std::deque< EnemyState > m_statesQueue;
   EnemyState m_currentState;

   // helper timer
   Timer m_timer;

   // total HP
   int32_t m_maxHP;

   // current weapon
   std::unique_ptr< Weapon > m_weapon;

   std::vector< glm::vec2 > m_positions{glm::vec2(0.5f, 0.5f), glm::vec2(0.0f, -0.5f), glm::vec2(-0.5f, 0.0f),
                                        glm::vec2(0.0f, 0.5f), glm::vec2(0.5f, 0.0f),  glm::vec2(-0.5f, -0.5f)};

   void
   Shoot();
   void
   ChasePlayer();
   void
   ReturnToInitialPosition();
   void
   ClearPositions();
   void
   SetTargetShootPosition(const glm::vec2& pos);
};
