#include <Enemy.hpp>
#include <Game.hpp>
#include <Player.hpp>
#include <Window.hpp>

Player::Player(const glm::vec2& position, const std::string& name)
   : GameObject(position, {32, 32}, name), m_name(name), m_velocity(0.0f, 0.0f), m_speed(0.0005f), m_maxHP(100), m_currentHP(m_maxHP)
{
   m_globalPosition = position;
   m_weapons[0] = std::make_unique< SniperRifle >();
   m_weapons[1] = std::make_unique< Glock >();

   m_currentWeapon = m_weapons.at(0).get();
}

void
Player::CreateSprite(const glm::vec2& position, const glm::ivec2& size, const std::string& fileName)
{
   m_collision = m_sprite.SetSpriteTextured(position, size, fileName);
   m_centeredGlobalPosition = m_sprite.GetCenteredPosition();
   m_localPosition = glm::ivec2(position.x, -position.y);

   m_program.LoadDefault();
}

void
Player::LoadShaders(const std::string& shaderFile)
{
   m_program.LoadShaders("../Shaders//" + shaderFile + "_vs.glsl", "../Shaders//" + shaderFile + "_fs.glsl");
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
   if (glm::length(glm::vec2(bulletPosition - m_centeredLocalPosition)) < (m_sprite.GetSize().x) / 2.5f)
   {
      if (enemyShooting)
      {
         m_currentHP -= enemy->GetDmg();
         m_sprite.SetColor(glm::vec3(1.0f, 0.0f, 0.0f));
      }
      return false;
   }
   return true;
}

glm::vec2
Player::GetScreenPosition() const
{
   glm::vec4 screenPosition = Game::GetInstance().GetProjection() * glm::vec4(m_centeredGlobalPosition, 0.0f, 1.0f);
   return glm::vec2(screenPosition.x, screenPosition.y);
}

void
Player::Render(const Shaders& program)
{
#pragma region CURSOR_MATH

   glm::vec2 cursorPos = Game::GetInstance().GetCursorScreenPosition();

   glm::vec4 tmpPos = Game::GetInstance().GetProjection() * glm::vec4(m_centeredGlobalPosition, 0.0f, 1.0f);
   float angle = -glm::degrees(glm::atan(tmpPos.y - cursorPos.y, tmpPos.x - cursorPos.x));

#pragma endregion
   m_sprite.Rotate(angle + 90.0f);
   m_sprite.Render(m_program);
   m_sprite.SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
}

void
Player::Shoot()
{
   glm::ivec2 direction = static_cast< glm::ivec2 >(Game::GetInstance().GetCursor()) - m_localPosition;
   m_currentWeapon->Shoot(direction);
}

void
Player::SetLocalPosition(const glm::ivec2& pos)
{
   m_localPosition = pos;
}

void
Player::SetCenteredLocalPosition(const glm::ivec2& pos)
{
   m_centeredLocalPosition = pos;
   m_localPosition = glm::ivec2(m_centeredLocalPosition.x - m_sprite.GetSize().x / 2, m_centeredLocalPosition.y - m_sprite.GetSize().y / 2);
   m_globalPosition = Game::GetInstance().GetLevel().GetGlobalVec(m_localPosition);
   m_centeredGlobalPosition = Game::GetInstance().GetLevel().GetGlobalVec(m_centeredLocalPosition);
}

void
Player::SetGlobalPosition(const glm::vec2& pos)
{
   m_localPosition = pos;
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
