#include "Enemy.h"
#include "Game.h"
#include "Level.h"
#include "Timer.h"
#include "Weapon.h"

extern Timer* globalTimer;

Enemy::Enemy( const glm::vec2& pos, const glm::ivec2& size, const std::string& sprite )
    : GameObject( pos, size, sprite )
    , m_maxHP( 100 )
    , m_currentHP( m_maxHP )
    , m_visionRange( 500.0f )
    , m_weapon( std::make_unique< Glock >( ) )
    , m_combatStarted( false )
{
}

void Enemy::DealWithPlayer( )
{
    // calculate distance between enemy and player
    float length = glm::length( m_centeredGlobalPosition - Game::GetInstance( ).GetPlayer( ).GetCenteredGlobalPosition( ) );

    // player is enemy's sight of vision
    if( length <= m_visionRange )
    {
        Shoot( );
        SetPlayerPos( Game::GetInstance( ).GetPlayer( ).GetCenteredLocalPosition( ) );
    }
    // player is out of range, clear enemy's 'memory'
    else
    {
        ClearPositions( );
    }

    SetCenteredLocalPosition( Game::GetInstance( ).GetLevel( ).GetLocalVec( m_centeredGlobalPosition ) );
    SetLocalPosition( Game::GetInstance( ).GetLevel( ).GetLocalVec( m_globalPosition ) );
}

void Enemy::Hit( int dmg )
{
    //currentHP -= dmg;
    SetColor( { 1.0f, 0.0f, 0.0f } );
}

bool Enemy::Visible( ) const
{
    if( m_currentHP <= 0 )
        return false;

    return true;
}

void Enemy::SetPlayerPos( const glm::vec2& playerPos )
{
    m_timer.ToggleAndAccumulate( );

    // make an enemy more human-like, prevent him from being perfect shooter
    // every half a second his aim will move a bit
    if( m_timer.GetAccumulator( ) > 0.5f )
    {
        m_timer.ResetAccumulator( );

        // compute small offset value which simulates the 'aim wiggle'
        int xOffset = rand( ) % Game::GetInstance( ).GetPlayer( ).GetSize( ).x + ( -Game::GetInstance( ).GetPlayer( ).GetSize( ).x / 2 );
        int yOffset = rand( ) % Game::GetInstance( ).GetPlayer( ).GetSize( ).y + ( -Game::GetInstance( ).GetPlayer( ).GetSize( ).y / 2 );

        m_playerPosition = ( playerPos + glm::vec2( xOffset, yOffset ) );
        m_combatStarted  = true;
    }
}

void Enemy::Shoot( )
{
    // prevent enemy shooting to some random position
    if( m_combatStarted )
    {
        glm::ivec2 collided = Game::GetInstance( ).CheckBulletCollision( this, Game::GetInstance( ).GetLevel( ).GetGlobalVec( m_playerPosition ), m_weapon->GetRange( ) );

        // if we hit anything draw a line
        if( collided != glm::ivec2( 0, 0 ) )
        {
            Game::GetInstance( ).DrawLine( m_centeredGlobalPosition, Game::GetInstance( ).GetLevel( ).GetGlobalVec( collided ) );
        }
    }
}

void Enemy::ClearPositions( )
{
    m_playerPosition = glm::vec2( 0.0f, 0.0f );
    m_timer.ResetAccumulator( );
    m_combatStarted = false;
}
