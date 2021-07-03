#pragma once

#include "Common.hpp"
#include "GameObject.hpp"
#include "Logger.hpp"
#include "Shader.hpp"
#include "Sprite.hpp"
#include "Weapon.hpp"

#include <array>
#include <deque>

namespace dgame {

class Window;
class Enemy;

class Player : public GameObject
{
 public:
   explicit Player(Application& game, const glm::vec2& position, const glm::ivec2& size,
                   const std::string& sprite, const std::string& name = "Anonymous");
   ~Player() override = default;

   // check if player got git by enemy
   bool
   CheckCollision(const glm::ivec2& bulletPosition, Enemy const* enemy, bool enemyShooting = true);

   // load shaders with given name
   void
   LoadShaders(const std::string& shaderFile);

   // load shaders with given shader program
   void
   LoadShaders(const Shader& program);

   // set position on map
   void
   SetLocalPosition(const glm::ivec2& pos) override;

   // set centered (center of player's sprite) position on map
   void
   SetCenteredLocalPosition(const glm::ivec2& pos) override;

   // set position in OpenGL
   void
   SetGlobalPosition(const glm::vec2& position) override;

   // get centered (center of player's sprite) position on screen
   // using projection matrix from OpenGL
   glm::vec2
   GetScreenPosition() const;

   // get reload time of player's current weapon
   float
   GetReloadTime() const;

   // change player's current weapon
   void
   ChangeWepon(int32_t idx);

   // get player's current weapon range
   int32_t
   GetWeaponRange() const;

   // get player's current weapon's damage
   int32_t
   GetWeaponDmg() const;

   std::vector< std::string >
   GetWeapons() const;

   // shoot with current weapon
   void
   Shoot();

   /*void
   CreateSprite(const glm::vec2& position, const glm::ivec2& size, const std::string& fileName);*/

   void Hit(int32_t) override
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
   State m_currentState;

   // array of player's weapons
   std::array< std::unique_ptr< Weapon >, 3 > m_weapons;

   // current weapon
   Weapon* m_currentWeapon = nullptr;

   // player's max health
   int32_t m_maxHP = 100;

   Logger m_logger;
};

} // namespace dgame