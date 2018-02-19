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
    int32_t m_maxHP;

    // current HP
    int32_t m_currentHP;

    // position where enemy will shoot
    glm::vec2 m_playerPosition;

    // enemy's vision range
    float m_visionRange;

	float m_timeSinceCombatStarted = 0.0f;
	float m_timeSinceLastShot = 0.0f;
	float m_reactionTime = 0.2f;
    // current weapon
    std::unique_ptr< Weapon > m_weapon;

    // fight between enemy and player started
    bool m_combatStarted;

    std::vector<glm::vec2> m_positions {
        glm::vec2(0.5f, 0.5f),
        glm::vec2(0.5f, 0.5f),
        glm::vec2(0.5f, 0.0f),
        glm::vec2(0.0f, 0.5f),
        glm::vec2(0.5f, 0.0f),
        glm::vec2(0.0f, 0.5f),
        glm::vec2(0.0f, 0.5f),
        glm::vec2(0.0f, 0.5f),
        glm::vec2(1.0f, 0.5f)};
    
    int32_t m_CurrentAnimationIndex = 0;
    glm::vec2 m_counter{0.0f, 0.0f};
    bool reverse = false;

 public:
    Enemy( const glm::vec2& pos, const glm::ivec2& size, const std::string& sprite );
    virtual ~Enemy( ) = default;

    virtual bool Visible( ) const override;
    virtual void Hit( int32_t dmg ) override;
    virtual void DealWithPlayer( ) override;

    int32_t GetDmg( ) const
    {
        return m_weapon->GetDamage( );
    }

    void Shoot( );
    void ClearPositions( );
    void SetPlayerPos( const glm::vec2& pos );
    void Render( const Shaders& program ) override;
    void Animate();
};
