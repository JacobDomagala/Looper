#pragma once

#include <glm/glm.hpp>
#include <string>
#include <utility>

namespace dgame {

class Weapon
{
 public:
   Weapon(float range, int32_t dmg, int32_t ammoNum, bool hasAmmo, std::string&& name,
          float reloadTime)
      : m_range(range),
        m_dmg(dmg),
        m_ammoNum(ammoNum),
        m_hasAmmo(hasAmmo),
        m_name(std::move(name)),
        m_reloadTime(reloadTime)
   {
   }

   virtual ~Weapon() = default;
   Weapon&
   operator=(Weapon const&) = default;
   Weapon&
   operator=(Weapon&&) = default;
   Weapon(Weapon&) = default;
   Weapon(Weapon&&) = default;

   [[nodiscard]] std::string
   GetName() const
   {
      return m_name;
   }

   [[nodiscard]] int32_t
   GetAmmoNum() const
   {
      return m_ammoNum;
   }

   [[nodiscard]] float
   GetReloadTime() const
   {
      return m_reloadTime;
   }

   [[nodiscard]] float
   GetRange() const
   {
      return m_range;
   }

   [[nodiscard]] int32_t
   GetDamage() const
   {
      return m_dmg;
   }

   virtual void
   Shoot(const glm::vec2& direction) = 0;

 protected:
   float m_range;
   int32_t m_dmg;
   int32_t m_ammoNum;
   bool m_hasAmmo;
   std::string m_name;
   float m_reloadTime;
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

} // namespace dgame