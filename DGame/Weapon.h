#pragma once

#include <utility>

#include <utility>

#include "Common.h"

class Weapon
{
 protected:
    const int32_t         m_range;
    int32_t         m_dmg;
    int32_t         m_ammoNum;
    bool        m_hasAmmo;
    const std::string m_name;
    float       m_reloadTime;

    Weapon( int32_t range, int32_t dmg, int32_t ammoNum, bool hasAmmo, std::string name, float reloadTime )
        : m_range( range )
        , m_dmg( dmg )
        , m_ammoNum( ammoNum )
        , m_hasAmmo( hasAmmo )
        , m_name( std::move( std::move( name ) ) )
        , m_reloadTime( reloadTime )
    {
    }

 public:
    virtual ~Weapon( ) = default;

    std::string GetName( ) const
    {
        return m_name;
    }
    int32_t GetAmmoNum( ) const
    {
        return m_ammoNum;
    }
    float GetReloadTime( ) const
    {
        return m_reloadTime;
    }
    constexpr int32_t GetRange( ) const
    {
        return m_range;
    }
    int32_t GetDamage( ) const
    {
        return m_dmg;
    }
    virtual void Shoot( const glm::vec2& direction ) = 0;
};

class SniperRifle : public Weapon
{

 public:
    SniperRifle( )
        : Weapon( 3000, 80, 10, true, { "sniper rifle" }, 1.0f )
    {
    }
    ~SniperRifle( ) override = default;

    void Shoot( const glm::vec2& /*direction*/ ) override
    {
    }
};

class Glock : public Weapon
{
 public:
    Glock( )
        : Weapon( 100, 10, 20, true, { "glock" }, 0.3f )
    {
    }
    ~Glock( ) override = default;

    void Shoot( const glm::vec2& /*direction*/ ) override
    {
    }
};
