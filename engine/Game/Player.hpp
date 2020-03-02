#pragma once

#include <Common.hpp>
#include <GameObject.hpp>
#include <Logger.hpp>
#include <Shaders.hpp>
#include <Sprite.hpp>
#include <Weapon.hpp>

#include <array>

class Window;
class Enemy;

class Player : public GameObject
{
 public:
   explicit Player(Game& game, const glm::vec2& position, const glm::ivec2& size, const std::string& sprite,
                   const std::string& name = "Anonymous");
   ~Player() = default;

   // check if player got git by enemy
   bool
   CheckCollision(const glm::ivec2& bulletPosition, Enemy const* enemy, bool enemyShooting = true);

   // load shaders with given name
   void
   LoadShaders(const std::string& shaderFile);

   // load shaders with given shader program
   void
   LoadShaders(const Shaders& program);

   // set position on map
   void
   SetLocalPosition(const glm::ivec2& pos);

   // set centered (center of player's sprite) position on map
   void
   SetCenteredLocalPosition(const glm::ivec2& pos);

   // set position in OpenGL
   void
   SetGlobalPosition(const glm::vec2& position);

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

   // shoot with current weapon
   void
   Shoot();

   void
   CreateSprite(const glm::vec2& position, const glm::ivec2& size, const std::string& fileName);

   void
   Render(Window& window);

   // draw player
   void
   Render(Window& window, const Shaders& program) override;

   void Hit(int32_t) override
   {
   }

   void
   DealWithPlayer() override
   {
   }

 private:
   struct State
   {
      // speed at which the player moves
      // (any moving imparing effects should lower this value)
      float m_speed;

      // player's current health
      int32_t m_currentHP;

      // player's velocity
      glm::vec2 m_velocity;
   };

   std::array< State, NUM_FRAMES_TO_SAVE > m_previousStates;
   State m_currentState;

   // name of the player
   std::string m_name;

   // array of player's weapons
   std::array< std::unique_ptr< Weapon >, 3 > m_weapons;

   // current weapon
   Weapon* m_currentWeapon;

   // player's max health
   int32_t m_maxHP;


   Logger m_logger;
};
