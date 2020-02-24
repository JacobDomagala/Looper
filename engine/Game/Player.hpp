#pragma once

#include <Common.hpp>
#include <GameObject.hpp>
#include <Shaders.hpp>
#include <Sprite.hpp>
#include <Weapon.hpp>
#include <array>

class Enemy;

class Player
{
   // name of the player
   std::string m_name;

   // local position with origin in upper left corner
   glm::ivec2 m_localPosition;

   // local position with origin in the center of sprite
   glm::ivec2 m_centeredLocalPosition;

   // global position in the origin in the upper left corner
   glm::vec2 m_globalPosition;

   // global position in the origin in the center of sprite
   glm::vec2 m_centeredGlobalPosition;

   // player's velocity
   glm::vec2 m_velocity;

   // speed at which the player moves
   // (any moving imparing effects should lower this value)
   float m_speed;

   // player's sprite
   Sprite m_sprite;

   // shader program to render player with
   Shaders m_program;

   // array of player's weapons
   std::array< std::unique_ptr< Weapon >, 3 > m_weapons;

   // current weapon
   Weapon* m_currentWeapon;

   // player's max health
   int32_t m_maxHP;

   // player's current health
   int32_t m_currentHP;

   // pixel values of player's sprite used for detailed collision
   std::unique_ptr< byte_vec4 > m_collision;

 public:
   explicit Player(const glm::vec2& position = glm::vec2(0.0f, 0.0f), const std::string& name = "Anonymous");
   ~Player() = default;

   // set sprite, update positions and load shader program
   void
   CreateSprite(const glm::vec2& position = glm::vec2(0.0f, 0.0f), const glm::ivec2& size = glm::ivec2(32, 32),
                const std::string& fileName = ".\\Default.png");

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

   // get position in OpenGL
   glm::vec2
   GetGlobalPosition() const;

   // get centered (center of player's sprite) position in OpenGL
   glm::vec2
   GetCenteredGlobalPosition() const;

   // get centered (center of player's sprite) position on screen
   // using projection matrix from OpenGL
   glm::vec2
   GetScreenPosition() const;

   // get centered (center of player's sprite) position on screen in pixels
   glm::ivec2
   GetScreenPositionPixels() const;

   // get position on map
   glm::ivec2
   GetLocalPosition() const;

   // get centered (center of player's sprite) position on map
   glm::ivec2
   GetCenteredLocalPosition() const;

   // get player's size (width and height)
   glm::ivec2
   GetSize() const;

   // move player in OpenGL space
   void
   Move(const glm::vec2& vector);

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

   // draw player
   void
   Draw();

   // shoot with current weapon
   void
   Shoot();
};
