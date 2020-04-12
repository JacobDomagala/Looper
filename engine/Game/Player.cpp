#include <Enemy.hpp>
#include <Game.hpp>
#include <Player.hpp>
#include <Window.hpp>

Player::Player(Context& game, const glm::vec2& position, const glm::ivec2& size, const std::string& sprite, const std::string& name)
   : GameObject(game, position, size, sprite)
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

void
Player::CreateSprite(const glm::vec2& position, const glm::ivec2& size, const std::string& fileName)
{
   m_collision = m_sprite.SetSpriteTextured(position, size, fileName);
   GameObject::m_currentState.m_centeredGlobalPosition = m_sprite.GetCenteredPosition();
   GameObject::m_currentState.m_localPosition = glm::ivec2(position.x, -position.y);

   m_program.LoadDefault();
}

void
Player::LoadShaders(const std::string& shaderFile)
{
   m_program.LoadShaders(shaderFile);
}

void
Player::LoadShaders(const Shaders& program)
{
   m_program = program;
}

bool
Player::CheckCollision(const glm::ivec2& bulletPosition, Enemy const* enemy, bool enemyShooting)
{
   // if the bullet is inside collision zone then player got hit
   if (glm::length(glm::vec2(bulletPosition - GameObject::m_currentState.m_centeredLocalPosition)) < (m_sprite.GetSize().x) / 2.5f)
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
   glm::vec4 screenPosition = m_contextHandle.GetProjection() * glm::vec4(GameObject::m_currentState.m_centeredGlobalPosition, 0.0f, 1.0f);
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
      if (m_contextHandle.IsGame())
      {
         auto gameHandle = ConvertToGameHandle();
         if (!gameHandle->IsReverse())
         {
            const auto cursorPos = gameHandle->GetCursorScreenPosition();
            const auto screenPosition = gameHandle->GetProjection() * gameHandle->GetViewMatrix()
                                        * glm::vec4(GameObject::m_currentState.m_centeredGlobalPosition, 0.0f, 1.0f);

            m_currentState.m_viewAngle = -glm::degrees(glm::atan(screenPosition.y - cursorPos.y, screenPosition.x - cursorPos.x));
         }
      }

      m_sprite.Rotate(m_currentState.m_viewAngle + 90.0f);
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

   glm::ivec2 direction = static_cast< glm::ivec2 >(gameHandle->GetCursor()) - GameObject::m_currentState.m_localPosition;
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
   GameObject::m_currentState.m_localPosition = glm::ivec2(GameObject::m_currentState.m_centeredLocalPosition.x - m_sprite.GetSize().x / 2,
                                                           GameObject::m_currentState.m_centeredLocalPosition.y - m_sprite.GetSize().y / 2);
   GameObject::m_currentState.m_globalPosition = m_contextHandle.GetLevel().GetGlobalVec(GameObject::m_currentState.m_localPosition);
   GameObject::m_currentState.m_centeredGlobalPosition =
      m_contextHandle.GetLevel().GetGlobalVec(GameObject::m_currentState.m_centeredLocalPosition);
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
   m_currentWeapon = m_weapons.at(idx).get();
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