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
    m_timer.ToggleTimer( );
}

void Enemy::DealWithPlayer( )
{
    // calculate distance between enemy and player
    float length = glm::length( m_centeredGlobalPosition - Game::GetInstance( ).GetPlayer( ).GetCenteredGlobalPosition( ) );

    auto collided = Game::GetInstance( ).IsPlayerInVision( this, m_visionRange );
    //auto collided = Game::GetInstance( ).CheckBulletCollision( this, Game::GetInstance( ).GetPlayer().GetCenteredGlobalPosition(), length );
    //auto length = glm::length(glm::vec2(collided.x, collided.y));
    // player is enemy's sight of vision
    m_timer.ToggleTimer( );
    auto timeElapsed = m_timer.GetDeltaTime( );

    if( collided )
    {
        if( m_combatStarted )
        {
            m_timeSinceCombatStarted += timeElapsed;

            if( m_timeSinceCombatStarted > m_reactionTime )
            {
                Shoot( );
                SetPlayerPos( Game::GetInstance( ).GetPlayer( ).GetCenteredLocalPosition( ) );
            }
        }
        else
        {
            SetPlayerPos( Game::GetInstance( ).GetPlayer( ).GetCenteredLocalPosition( ) );
        }

        m_timer.ResetAccumulator( );
        // TODO: CHASE PLAYER
    }
    // player is out of range, clear enemy's 'memory'
    else
    {
        ClearPositions( );
    }

    SetCenteredLocalPosition( Game::GetInstance( ).GetLevel( ).GetLocalVec( m_centeredGlobalPosition ) );
    SetLocalPosition( Game::GetInstance( ).GetLevel( ).GetLocalVec( m_globalPosition ) );
}

void Enemy::Hit( int32_t dmg )
{
    //currentHP -= dmg;
    SetColor( { 1.0f, 0.0f, 0.0f } );
}

bool Enemy::Visible( ) const
{
    return ( m_currentHP > 0 );
}

void Enemy::SetPlayerPos( const glm::vec2& playerPos )
{
    // compute small offset value which simulates the 'aim wiggle'
    int32_t xOffset = rand( ) % Game::GetInstance( ).GetPlayer( ).GetSize( ).x + ( -Game::GetInstance( ).GetPlayer( ).GetSize( ).x / 2 );
    int32_t yOffset = rand( ) % Game::GetInstance( ).GetPlayer( ).GetSize( ).y + ( -Game::GetInstance( ).GetPlayer( ).GetSize( ).y / 2 );

    m_playerPosition = ( playerPos + glm::vec2( xOffset, yOffset ) );
    m_combatStarted  = true;
}

void Enemy::Shoot( )
{
    auto timeElapsed = m_timer.GetDeltaTime( );
    m_timeSinceLastShot += timeElapsed;
    // prevent enemy shooting to some random position
    /*if( m_combatStarted )
    {*/
    if( m_timeSinceLastShot > m_weapon->GetReloadTime( ) ) //0.5f )
    {
        auto collided = Game::GetInstance( ).CheckBulletCollision( this, Game::GetInstance( ).GetLevel( ).GetGlobalVec( m_playerPosition ), m_weapon->GetRange( ) );

        // if we hit anything draw a line
        if( collided.first != glm::ivec2( 0, 0 ) )
        {
            Game::GetInstance( ).DrawLine( m_centeredGlobalPosition, Game::GetInstance( ).GetLevel( ).GetGlobalVec( collided.first ) );
        }

        m_timeSinceLastShot = 0.0f;
    }
}

void Enemy::ClearPositions( )
{
    m_playerPosition         = glm::vec2( 0.0f, 0.0f );
    m_combatStarted          = false;
    m_timeSinceCombatStarted = 0.0f;
    m_timeSinceLastShot      = 0.0f;
}

void Enemy::Render( const Shaders& program )
{
    if( !m_combatStarted )
    {
        Animate( );
    }

    GameObject::Render( program );
}

void Enemy::Animate( )
{
    auto deltaTime            = Game::GetInstance( ).GetDeltaTime( );
    auto currentAnimation     = m_positions.at( m_CurrentAnimationIndex );
    auto currentAnimationStep = glm::vec2( currentAnimation.x * deltaTime, currentAnimation.y * deltaTime );

    if( !reverse )
    {
        auto nextStep = m_counter + currentAnimationStep;

        if( ( nextStep.x <= currentAnimation.x ) && ( nextStep.y <= currentAnimation.y ) )
        {
            m_counter += currentAnimationStep;
        }
        else if( ( nextStep.x >= currentAnimation.x ) && ( nextStep.y >= currentAnimation.y ) )
        {
            m_counter = currentAnimation;

            Move( m_counter, false );

            ++m_CurrentAnimationIndex;
            if( m_CurrentAnimationIndex == m_positions.size( ) )
            {
                reverse = true;
                --m_CurrentAnimationIndex;
            }

            m_counter = glm::vec2( );
        }
    }
    else // reverse
    {
        currentAnimation *= -1.0f;
        currentAnimationStep *= -1.0f;

        auto nextStep = m_counter + currentAnimationStep;

        if( ( nextStep.x >= currentAnimation.x ) && ( nextStep.y >= currentAnimation.y ) )
        {
            m_counter += currentAnimationStep;
        }
        else if( ( nextStep.x <= currentAnimation.x ) && ( nextStep.y <= currentAnimation.y ) )
        {
            m_counter = currentAnimation;

            Move( m_counter, false );

            --m_CurrentAnimationIndex;
            if( m_CurrentAnimationIndex == -1 )
            {
                reverse = false;
                ++m_CurrentAnimationIndex;
            }

            m_counter = glm::vec2( );
        }
    }

    Move( m_counter, false );
}
