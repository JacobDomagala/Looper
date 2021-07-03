#include "Renderer.hpp"
#include <Enemy.hpp>
#include <Game.hpp>
#include <Player.hpp>
#include <Window.hpp>

namespace dgame {

Player::Player(Application& game, const glm::vec2& position, const glm::ivec2& size,
               const std::string& sprite, const std::string& name)
   : GameObject(game, position, size, sprite, TYPE::PLAYER)
{
   m_logger.Init("Player");

   m_name = name;
   m_currentState.m_velocity = {0.0f, 0.0f};
   m_currentState.m_speed = 0.0005f;
   m_maxHP = 100;
   m_currentState.m_currentHP = m_maxHP;
   // GameObject::m_currentState.m_globalPosition = position;
   m_weapons[0] = std::make_unique< SniperRifle >();
   m_weapons[1] = std::make_unique< Glock >();

   m_currentWeapon = m_weapons.at(0).get();
}

// void
// Player::CreateSprite(const glm::vec2& position, const glm::ivec2& size, const std::string&
// fileName)
//{
//   m_collision = m_sprite.SetSpriteTextured(position, size, fileName);
//   GameObject::m_currentState.m_centeredGlobalPosition = m_sprite.GetPosition();
//   GameObject::m_currentState.m_localPosition = glm::ivec2(position.x, -position.y);
//}

void
Player::LoadShaders(const std::string&)
{
}

void
Player::LoadShaders(const Shader&)
{
}

bool
Player::CheckCollision(const glm::ivec2& bulletPosition, Enemy const* enemy, bool enemyShooting)
{
   // if the bullet is inside collision zone then player got hit
   if (glm::length(glm::vec2(bulletPosition - GameObject::m_currentState.m_centeredLocalPosition))
       < (static_cast< float >(m_sprite.GetSize().x)) / 2.5f)
   {
      if (enemyShooting)
      {
         m_currentState.m_currentHP -= enemy->GetDmg();
         m_sprite.SetColor(glm::vec3(1.0f, 0.0f, 0.0f));
      }
      return false;
   }
   return true;
}

glm::vec2
Player::GetScreenPosition() const
{
   glm::vec4 screenPosition =
      m_appHandle.GetProjection()
      * glm::vec4(GameObject::m_currentState.m_centeredGlobalPosition, 0.0f, 1.0f);
   return glm::vec2(screenPosition.x, screenPosition.y);
}

void
Player::UpdateInternal(bool isReverse)
{
   if (isReverse)
   {
      m_currentState = m_statesQueue.back();
      m_statesQueue.pop_back();
   }
   else
   {
      if (m_appHandle.IsGame())
      {
         if (!isReverse)
         {
            const auto gameHandle = ConvertToGameHandle();
            const auto cursorPos = gameHandle->ScreenToGlobal(gameHandle->GetCursor());
            const auto spritePosition = GameObject::m_currentState.m_globalPosition;

            m_currentState.m_viewAngle =
               glm::atan(spritePosition.y - cursorPos.y, spritePosition.x - cursorPos.x);
         }
      }

      m_sprite.Rotate(m_currentState.m_viewAngle);
      m_sprite.SetColor(glm::vec3(1.0f, 1.0f, 1.0f));

      m_statesQueue.push_back(m_currentState);

      if (m_statesQueue.size() >= NUM_FRAMES_TO_SAVE)
      {
         m_statesQueue.pop_front();
      }
   }
}

void
Player::Shoot()
{
   auto gameHandle = ConvertToGameHandle();

   glm::ivec2 direction = static_cast< glm::ivec2 >(gameHandle->GetCursor())
                          - GameObject::m_currentState.m_localPosition;
   m_currentWeapon->Shoot(direction);
}

void
Player::SetLocalPosition(const glm::ivec2& pos)
{
   GameObject::m_currentState.m_localPosition = pos;
}

void
Player::SetCenteredLocalPosition(const glm::ivec2& pos)
{
   GameObject::m_currentState.m_centeredLocalPosition = pos;
   GameObject::m_currentState.m_localPosition =
      glm::ivec2(GameObject::m_currentState.m_centeredLocalPosition.x - m_sprite.GetSize().x / 2,
                 GameObject::m_currentState.m_centeredLocalPosition.y - m_sprite.GetSize().y / 2);
   GameObject::m_currentState.m_globalPosition =
      m_appHandle.GetLevel().GetGlobalVec(GameObject::m_currentState.m_localPosition);
   GameObject::m_currentState.m_centeredGlobalPosition =
      m_appHandle.GetLevel().GetGlobalVec(GameObject::m_currentState.m_centeredLocalPosition);
}

void
Player::SetGlobalPosition(const glm::vec2& pos)
{
   GameObject::m_currentState.m_localPosition = pos;
}

float
Player::GetReloadTime() const
{
   return m_currentWeapon->GetReloadTime();
}

void
Player::ChangeWepon(int32_t idx)
{
   m_currentWeapon = m_weapons.at(static_cast< size_t >(idx)).get();
}

int32_t
Player::GetWeaponRange() const
{
   return m_currentWeapon->GetRange();
}

int32_t
Player::GetWeaponDmg() const
{
   return m_currentWeapon->GetDamage();
}

std::vector< std::string >
Player::GetWeapons() const
{
   std::vector< std::string > weapons{};

   for (const auto& weapon : m_weapons)
   {
      if (weapon)
      {
         weapons.push_back(weapon->GetName());
      }
   }

   return weapons;
}

} // namespace dgame
