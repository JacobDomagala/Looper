#include <enemy.hpp>
#include <game.hpp>
#include <player.hpp>
#include <renderer/window/window.hpp>

namespace looper {

Player::Player(Application& game, const glm::vec2& position, const glm::ivec2& size,
               const std::string& sprite, const std::string& name)
   : GameObject(game, position, size, sprite, TYPE::PLAYER)
{
   m_name = name;
   m_currentState.m_velocity = {0.0f, 0.0f};
   m_currentState.m_speed = 0.0005f;

   m_currentState.m_currentHP = m_maxHP;
   // m_currentGameObjectState.m_position = position;
   m_weapons[0] = std::make_unique< SniperRifle >();
   m_weapons[1] = std::make_unique< Glock >();

   // NOLINTNEXTLINE
   m_currentWeapon = m_weapons.at(0).get();
}

bool
Player::CheckCollision(const glm::vec2& bulletPosition, Enemy const* enemy, bool enemyShooting)
{
   // if the bullet is inside collision zone then player got hit
   if (glm::length(bulletPosition - m_currentGameObjectState.m_centeredPosition)
       < (static_cast< float >(m_sprite.GetSize().x)) / 2.5f)
   {
      if (enemyShooting)
      {
         m_currentState.m_currentHP -= enemy->GetDmg();
         m_sprite.SetColor({1.0f, 0.0f, 0.0f, 0.75f});
      }
      return false;
   }
   return true;
}

glm::vec2
Player::GetScreenPosition() const
{
   const glm::vec4 screenPosition = m_appHandle.GetProjection()
                              * glm::vec4(m_currentGameObjectState.m_centeredPosition, 0.0f, 1.0f);
   return {screenPosition.x, screenPosition.y};
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
         auto* const gameHandle = ConvertToGameHandle();
         const auto cursorPos = gameHandle->ScreenToGlobal(gameHandle->GetCursor());
         const auto spritePosition = m_currentGameObjectState.m_position;

         m_currentState.m_viewAngle =
            glm::atan(spritePosition.y - cursorPos.y, spritePosition.x - cursorPos.x);
      }

      m_sprite.Rotate(m_currentState.m_viewAngle);
      m_sprite.SetColor({1.0f, 1.0f, 1.0f, 0.75f});

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
   auto* gameHandle = ConvertToGameHandle();

   const auto direction = gameHandle->GetCursor() - m_currentGameObjectState.m_position;
   m_currentWeapon->Shoot(direction);
}

void
Player::SetPosition(const glm::vec2& position)
{
   m_currentGameObjectState.m_position = position;
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

float
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

} // namespace looper
