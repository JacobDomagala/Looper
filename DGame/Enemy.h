#pragma once

#include "Common.h"
#include "GameObject.h"
#include "Timer.h"
#include "Weapon.h"

class Enemy : public GameObject
{
    // helper timer
    Timer m_timer;

    // total HP
    int m_maxHP;

    // current HP
    int m_currentHP;

    // position where enemy will shoot
    glm::vec2 m_playerPosition;

    // enemy's vision range
    float m_visionRange;

    // current weapon
    std::unique_ptr< Weapon > m_weapon;

    // fight between enemy and player started
    bool m_combatStarted;

 public:
    Enemy( const glm::vec2& pos, const glm::ivec2& size, const std::string& sprite );
    virtual ~Enemy( ) = default;

    virtual bool Visible( ) const override;
    virtual void Hit( int dmg ) override;
    virtual void DealWithPlayer( ) override;

    int GetDmg( ) const
    {
        return m_weapon->GetDamage( );
    }

    void Shoot( );
    void ClearPositions( );
    void SetPlayerPos( const glm::vec2& pos );
};
