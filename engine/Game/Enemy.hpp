#pragma once

#include "Animatable.hpp"
#include "Common.hpp"
#include "GameObject.hpp"
#include "Timer.hpp"
#include "Weapon.hpp"

#include <deque>
#include <vector>

class Context;
class Window;

class Enemy : public GameObject, public Animatable
{
 public:
   Enemy(Context& context, const glm::vec2& pos, const glm::ivec2& size, const std::string& textureName,
         const std::list< AnimationPoint >& keypoints = {},
         Animatable::ANIMATION_TYPE animationType = Animatable::ANIMATION_TYPE::REVERSABLE);
   ~Enemy() override = default;

   bool
   Visible() const override;

   void
   Hit(int32_t dmg) override;

   void
   DealWithPlayer() override;

   void
   Render(Shaders& program) override;

   std::string
   GetWeapon() const;

   int32_t
   GetDmg() const;

 private:
   enum class ACTION
   {
      IDLE,
      CHASING_PLAYER,
      SHOOTING,
      RETURNING
   };

 private:
   void
   UpdateInternal(bool isReverse) override;
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

 private:
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
      float m_visionRange = 0.0f;

      bool m_combatStarted = false;

      int m_currentNodeIdx;
   };

   std::deque< EnemyState > m_statesQueue;
   EnemyState m_currentState;

   // helper timer
   Timer m_timer;

   // total HP
   int32_t m_maxHP = 100;

   // current weapon
   std::unique_ptr< Weapon > m_weapon;
};
