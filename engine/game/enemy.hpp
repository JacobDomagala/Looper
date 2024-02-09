#pragma once

#include "animatable.hpp"
#include "common.hpp"
#include "game_object.hpp"
#include "utils/time/timer.hpp"
#include "weapon.hpp"

#include <vector>

namespace looper {

class Application;

class Enemy : public GameObject, public Animatable
{
 public:
   Enemy(Application* context, const glm::vec2& pos, const glm::ivec2& size,
         const std::string& textureName, const std::vector< AnimationPoint >& keypoints = {},
         Animatable::ANIMATION_TYPE animationType = Animatable::ANIMATION_TYPE::REVERSABLE);

   Enemy() = default;

   void Setup(Application* context, const glm::vec2& pos, const glm::ivec2& size,
         const std::string& textureName, const std::vector< AnimationPoint >& keypoints = {},
         Animatable::ANIMATION_TYPE animationType = Animatable::ANIMATION_TYPE::REVERSABLE);

   [[nodiscard]] bool
   Visible() const override;

   void
   Hit(int32_t dmg) override;

   void
   DealWithPlayer();

   [[nodiscard]] glm::ivec2
   GetInitialPosition() const;

 private:
   enum class ACTION
   {
      IDLE,
      CHASING_PLAYER,
      SHOOTING,
      RETURNING
   };

   void
   UpdateInternal(bool isReverse) override;

   /**
    * @brief Move Enemy to target position
    * @param[in]: targetPosition target position
    * @param[in]: exactPosition whether Enemy wants to actually stand in targetPosition,
    * e.g. when getting back to its initial position
    *
    * @return Whether the destination is reached
    */
   bool
   MoveToPosition(const glm::vec2& targetPosition, bool exactPosition = false);

   void
   EnemyMove(const glm::vec2& moveBy);

   void
   Shoot();

   void
   ChasePlayer();

   void
   ReturnToInitialPosition();

   void
   ClearPositions();

   void
   SetTargetShootPosition(const glm::vec2& targetPosition);

   struct State
   {
      ACTION action_ = ACTION::IDLE;
      int32_t currentHP_ = {};
      glm::vec2 targetShootPosition_ = {};
      glm::vec2 targetMovePosition_ = {};

      glm::vec2 lastPlayersPos_ = {};

      bool isChasingPlayer_ = false;
      bool isAtInitialPos_ = true;

      time::microseconds timeSinceCombatEnded_ = time::microseconds(0);
      float timeSinceCombatStarted_ = 0.0f;
      float timeSinceLastShot_ = 0.0f;
      float reactionTime_ = 0.1f;
      float movementSpeed_ = 0.5f;
      float visionRange_ = 0.0f;

      bool combatStarted_ = false;
      float viewAngle_ = {};
   };

   StateList< State > enemyStatesQueue_;
   State currentState_;

   // helper timer
   time::Timer timer_;

   // total HP
   int32_t maxHP_ = 100;

   glm::vec2 initialPosition_ = {};
};

} // namespace looper
