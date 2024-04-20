#pragma once

#include <glm/glm.hpp>
#include <string>
#include <utility>

namespace looper {

class Weapon
{
 public:
   Weapon(float range, int32_t dmg, int32_t ammoNum, bool hasAmmo, std::string&& name,
          float reloadTime)
      : range_(range),
        dmg_(dmg),
        ammoNum_(ammoNum),
        hasAmmo_(hasAmmo),
        name_(std::move(name)),
        reloadTime_(reloadTime)
   {
   }

   virtual ~Weapon() = default;
   Weapon&
   operator=(Weapon const&) = default;
   Weapon&
   operator=(Weapon&&) = default;
   Weapon(Weapon&) = default;
   Weapon(Weapon&&) = default;

   [[nodiscard]] const std::string&
   GetName() const
   {
      return name_;
   }

   [[nodiscard]] int32_t
   GetAmmoNum() const
   {
      return ammoNum_;
   }

   [[nodiscard]] float
   GetReloadTime() const
   {
      return reloadTime_;
   }

   [[nodiscard]] float
   GetRange() const
   {
      return range_;
   }

   [[nodiscard]] int32_t
   GetDamage() const
   {
      return dmg_;
   }

   virtual void
   Shoot(const glm::vec2& direction) = 0;

 protected:
   float range_;
   int32_t dmg_;
   int32_t ammoNum_;
   bool hasAmmo_;
   std::string name_;
   float reloadTime_;
};

class SniperRifle : public Weapon
{
 public:
   SniperRifle() : Weapon(1000.0f, 80, 10, true, {"sniper rifle"}, 1.0f)
   {
   }

   void
   Shoot(const glm::vec2& /*direction*/) override
   {
   }
};

class Glock : public Weapon
{
 public:
   Glock() : Weapon(300.0f, 10, 20, true, {"glock"}, 0.3f)
   {
   }

   void
   Shoot(const glm::vec2& /*direction*/) override
   {
   }
};

} // namespace looper