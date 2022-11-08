#pragma once

#include "Common.hpp"
#include "GameObject.hpp"
#include "Logger.hpp"
#include "Shader.hpp"
#include "Sprite.hpp"
#include "Weapon.hpp"

#include <array>
#include <deque>

namespace looper {

class Window;
class Enemy;

class Player : public GameObject
{
 public:
   Player(Application& game, const glm::vec2& position, const glm::ivec2& size,
          const std::string& sprite, const std::string& name = "Anonymous");

   // check if player got git by enemy
   bool
   CheckCollision(const glm::vec2& bulletPosition, Enemy const* enemy, bool enemyShooting = true);

   // load shaders with given name
   void
   LoadShaders(const std::string& shaderFile);

   // load shaders with given shader program
   void
   LoadShaders(const Shader& program);

   // set position in OpenGL
   void
   SetPosition(const glm::vec2& position) override;

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

   void Hit(int32_t /*dmg*/) override
   {
   }

 private:
   void
   UpdateInternal(bool isReverse) override;

   struct State
   {
      // speed at which the player moves
      // (any moving imparing effects should lower this value)
      float m_speed;

      // player's current health
      int32_t m_currentHP;

      // player's velocity
      glm::vec2 m_velocity;

      // direction where player is looking
      float m_viewAngle;
   };

   std::deque< State > m_statesQueue;
   State m_currentState = {};

   // array of player's weapons
   std::array< std::unique_ptr< Weapon >, 3 > m_weapons;

   // current weapon
   Weapon* m_currentWeapon = nullptr;

   // player's max health
   int32_t m_maxHP = 100;

   Logger m_logger;
};

} // namespace looper