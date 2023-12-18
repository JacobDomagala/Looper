#include <enemy.hpp>
#include <game.hpp>
#include <player.hpp>
#include <renderer/window/window.hpp>

namespace looper {

Player::Player(Application* game, const glm::vec3& position, const glm::ivec2& size,
               const std::string& sprite, const std::string& name)
   : GameObject(game, position, size, sprite, ObjectType::PLAYER)
{
   name_ = name;
   currentState_.velocity_ = {0.0f, 0.0f};
   currentState_.speed_ = 0.05f;

   currentState_.currentHP_ = maxHP_;
   // currentGameObjectState_.position_ = position;
   weapons_[0] = std::make_unique< SniperRifle >();
   weapons_[1] = std::make_unique< Glock >();

   // NOLINTNEXTLINE
   currentWeapon_ = weapons_.at(0).get();
}

Player::Player(Application* game, const glm::vec2& position, const glm::ivec2& size,
               const std::string& sprite, const std::string& name)
   : Player(game, glm::vec3{position, 0.0f}, size, sprite, name)
{
}

Player::Player()
{
}

void
Player::Setup(Application* game, const glm::vec3& position, const glm::ivec2& size,
              const std::string& sprite, const std::string& name)
{
   GameObject::Setup(game, position, size, sprite, ObjectType::PLAYER);

   name_ = name;
   currentState_.velocity_ = {0.0f, 0.0f};
   currentState_.speed_ = 0.05f;

   currentState_.currentHP_ = maxHP_;
   // currentGameObjectState_.position_ = position;
   weapons_[0] = std::make_unique< SniperRifle >();
   weapons_[1] = std::make_unique< Glock >();

   // NOLINTNEXTLINE
   currentWeapon_ = weapons_.at(0).get();
}

bool
Player::CheckCollision(const glm::vec2& bulletPosition, Enemy const* /* enemy*/, bool enemyShooting)
{
   // if the bullet is inside collision zone then player got hit
   if (glm::length(bulletPosition - glm::vec2(currentGameObjectState_.centeredPosition_))
       < (static_cast< float >(sprite_.GetSize().x)) / 2.5f)
   {
      if (enemyShooting)
      {
         // currentState_.currentHP_ -= enemy->GetDmg();
         sprite_.SetColor({1.0f, 0.0f, 0.0f, 0.75f});
      }
      return false;
   }
   return true;
}

glm::vec2
Player::GetScreenPosition() const
{
   const glm::vec4 screenPosition =
      appHandle_->GetProjection()
      * glm::vec4(currentGameObjectState_.centeredPosition_, 0.0f, 1.0f);
   return {screenPosition.x, screenPosition.y};
}

void
Player::UpdateInternal(bool isReverse)
{
   if (isReverse)
   {
      currentState_ = statesQueue_.GetLastState();
   }
   else
   {
      if (appHandle_->IsGame())
      {
         auto* const gameHandle = ConvertToGameHandle();
         const auto cursorPos = gameHandle->ScreenToGlobal(gameHandle->GetCursor());
         const auto spritePosition = currentGameObjectState_.position_;

         currentState_.viewAngle_ =
            glm::atan(cursorPos.y - spritePosition.y, cursorPos.x - spritePosition.x);
      }

      sprite_.Rotate(currentState_.viewAngle_);
      sprite_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});

      statesQueue_.PushState(currentState_);
   }
}

void
Player::Shoot()
{
   auto* gameHandle = ConvertToGameHandle();

   const auto direction = gameHandle->GetCursor() - glm::vec2(currentGameObjectState_.position_);
   currentWeapon_->Shoot(direction);
}

void
Player::SetPosition(const glm::vec2& position)
{
   currentGameObjectState_.position_ = position;
}

float
Player::GetReloadTime() const
{
   return currentWeapon_->GetReloadTime();
}

void
Player::ChangeWepon(int32_t idx)
{
   currentWeapon_ = weapons_.at(static_cast< size_t >(idx)).get();
}

float
Player::GetWeaponRange() const
{
   return currentWeapon_->GetRange();
}

int32_t
Player::GetWeaponDmg() const
{
   return currentWeapon_->GetDamage();
}

std::vector< std::string >
Player::GetWeapons() const
{
   std::vector< std::string > weapons{};

   for (const auto& weapon : weapons_)
   {
      if (weapon)
      {
         weapons.push_back(weapon->GetName());
      }
   }

   return weapons;
}

} // namespace looper
