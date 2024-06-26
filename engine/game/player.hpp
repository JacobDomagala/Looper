#pragma once

#include "common.hpp"
#include "game_object.hpp"
#include "logger.hpp"
#include "renderer/sprite.hpp"
#include "weapon.hpp"

#include <array>

namespace looper {

class Enemy;

class Player : public GameObject
{
 public:
   Player(Application* game, const glm::vec3& position, const glm::ivec2& size,
          const std::string& sprite, const std::string& name = "Anonymous");

   Player(Application* game, const glm::vec2& position, const glm::ivec2& size,
          const std::string& sprite, const std::string& name = "Anonymous");

   Player() = default;

   void
   Setup(Application* game, const glm::vec2& position, const glm::ivec2& size,
         const std::string& sprite, const std::string& name = "Anonymous");

   // check if player got git by enemy
   bool
   CheckCollision(const glm::vec2& bulletPosition, Enemy const* enemy, bool enemyShooting = true);

   // get centered (center of player's sprite) position on screen
   // using projection matrix from OpenGL
   [[nodiscard]] glm::vec2
   GetScreenPosition() const;

   // get reload time of player's current weapon
   [[nodiscard]] float
   GetReloadTime() const;

   // change player's current weapon
   void
   ChangeWepon(int32_t idx);

   // get player's current weapon range
   [[nodiscard]] float
   GetWeaponRange() const;

   // get player's current weapon's damage
   [[nodiscard]] int32_t
   GetWeaponDmg() const;

   [[nodiscard]] std::vector< std::string >
   GetWeapons() const;

   // shoot with current weapon
   void
   Shoot();

   /*void
   CreateSprite(const glm::vec2& position, const glm::ivec2& size, const std::string& fileName);*/

 private:
   void
   UpdateInternal(bool isReverse) override;

   struct State
   {
      // speed at which the player moves
      // (any moving imparing effects should lower this value)
      float speed_;

      // player's current health
      int32_t currentHP_;

      // player's velocity
      glm::vec2 velocity_;

      // direction where player is looking
      float viewAngle_;
   };

   StateList< State > statesQueue_;
   State currentState_ = {};

   // array of player's weapons
   std::array< std::unique_ptr< Weapon >, 3 > weapons_;

   // current weapon
   Weapon* currentWeapon_ = nullptr;

   // player's max health
   int32_t maxHP_ = 100;
};

} // namespace looper