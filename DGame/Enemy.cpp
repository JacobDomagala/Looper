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
    , m_visionRange( 1000.0f )
    , m_weapon( std::make_unique< Glock >( ) )
    , m_combatStarted( false )
{
    m_timer.ToggleTimer( );
    m_initialPosition = m_centeredLocalPosition;
    m_currentNodeIdx  = Game::GetInstance( ).GetLevel( ).GetPathfinder( ).FindNodeIdx( m_centeredLocalPosition );
}

void Enemy::DealWithPlayer( )
{
    Game::GetInstance( ).RenderText( std::to_string( m_currentNodeIdx ), glm::vec2( 125.5f, 125.5f ), 1.0f, glm::vec3( 1.0f, 0.0f, 0.0f ) );

    auto collided = Game::GetInstance( ).IsPlayerInVision( this, static_cast< int32_t >( m_visionRange ) );

    m_timer.ToggleTimer( );

    // player in enemy's sight of vision
    if( collided )
    {
		m_currentState = STATE::SHOOTING;
        m_lastPlayersPos        = Game::GetInstance( ).GetPlayer( ).GetCenteredLocalPosition( );
        m_timeSinceCombatEnded  = 0.0f;
        m_CurrentAnimationIndex = 0;
        m_counter               = glm::vec2( 0.0f, 0.0f );
        reverse                 = false;

        if( m_combatStarted )
        {
            m_timeSinceCombatStarted += m_timer.GetDeltaTime( );

            if( m_timeSinceCombatStarted > m_reactionTime )
            {
                Shoot( );
                SetTargetShootPosition( Game::GetInstance( ).GetPlayer( ).GetCenteredLocalPosition( ) );
            }
        }
        else
        {
            SetTargetShootPosition( Game::GetInstance( ).GetPlayer( ).GetCenteredLocalPosition( ) );
        }

        m_timer.ResetAccumulator( );
    }
    // player is out of range, clear enemy's 'memory'
    else
    {
        m_timeSinceCombatEnded += m_timer.GetDeltaTime( );

        if( (m_currentState != STATE::IDLE) && (m_timeSinceCombatEnded < 3.0f) )
        {
			m_currentState = STATE::CHASING_PLAYER;
            m_isChasingPlayer = true;
            ChasePlayer( );
        }
        else
        {
            m_isChasingPlayer = false;
            if( !m_isAtInitialPos )
            {
				m_currentState = STATE::RETURNING;
                ReturnToInitialPosition( );
            }
            else
            {
				m_currentState = STATE::IDLE;
                ClearPositions( );
            }
        }
    }

    SetCenteredLocalPosition( Game::GetInstance( ).GetLevel( ).GetLocalVec( m_centeredGlobalPosition ) );
    SetLocalPosition( Game::GetInstance( ).GetLevel( ).GetLocalVec( m_globalPosition ) );
}

void Enemy::Hit( int32_t /*dmg*/ )
{
    //currentHP -= dmg;
    SetColor( { 1.0f, 0.0f, 0.0f } );
}

bool Enemy::Visible( ) const
{
    return ( m_currentHP > 0 );
}

void Enemy::SetTargetShootPosition( const glm::vec2& playerPos )
{
    // compute small offset value which simulates the 'aim wiggle'
    int32_t xOffset = rand( ) % Game::GetInstance( ).GetPlayer( ).GetSize( ).x + ( -Game::GetInstance( ).GetPlayer( ).GetSize( ).x / 2 );
    int32_t yOffset = rand( ) % Game::GetInstance( ).GetPlayer( ).GetSize( ).y + ( -Game::GetInstance( ).GetPlayer( ).GetSize( ).y / 2 );

    m_targetShootPosition = ( playerPos + glm::vec2( xOffset, yOffset ) );
    m_combatStarted       = true;
}

void Enemy::Shoot( )
{
    m_timeSinceLastShot += m_timer.GetDeltaTime( );

    Game::GetInstance( ).RenderText( "POW POW", glm::vec2( 128.0f, 64.0f ), 1.0f, glm::vec3( 0.0f, 0.1f, 0.4f ) );
    if( glm::length( static_cast< glm::vec2 >( Game::GetInstance( ).GetPlayer( ).GetCenteredLocalPosition( ) - m_centeredLocalPosition ) ) <= m_weapon->GetRange( ) )
    {
        if( m_timeSinceLastShot >= m_weapon->GetReloadTime( ) )
        {
            auto collided = Game::GetInstance( ).CheckBulletCollision( this, Game::GetInstance( ).GetLevel( ).GetGlobalVec( m_targetShootPosition ), m_weapon->GetRange( ) );

            // if we hit anything draw a line
            if( collided.first != glm::ivec2( 0, 0 ) )
            {
                Game::GetInstance( ).DrawLine( m_centeredGlobalPosition, Game::GetInstance( ).GetLevel( ).GetGlobalVec( collided.first ) );
            }

            m_timeSinceLastShot = 0.0f;
        }
    }
    else
    {
        ChasePlayer( );
    }
}

void Enemy::ChasePlayer( )
{
    auto playerPos = m_lastPlayersPos; // Game::GetInstance().GetPlayer().GetCenteredLocalPosition();
    auto moveBy    = 500.0f * Game::GetInstance( ).GetDeltaTime( );

    auto distanceToNode = glm::length( static_cast< glm::vec2 >( m_targetMovePosition - m_centeredLocalPosition ) );

    if( ( distanceToNode < 5.0f ) && ( distanceToNode != 0.0f ) )
    {
        Move( m_targetMovePosition - m_centeredLocalPosition, false );
        m_currentNodeIdx = Game::GetInstance( ).GetLevel( ).GetPathfinder( ).FindNodeIdx( m_centeredLocalPosition );
    }
    else if( m_targetMovePosition == m_centeredLocalPosition /*|| m_targetMovePosition == glm::ivec2(0, 0)*/ )
    {
        m_currentNodeIdx     = Game::GetInstance( ).GetLevel( ).GetPathfinder( ).FindNodeIdx( m_centeredLocalPosition );
        m_targetMovePosition = Game::GetInstance( ).GetLevel( ).GetPathfinder( ).GetNearestPosition( m_currentNodeIdx, playerPos );
        if( m_targetMovePosition != glm::ivec2( 0, 0 ) )
        {
            auto move  = m_targetMovePosition - m_centeredLocalPosition;
            auto nMove = glm::normalize( static_cast< glm::vec2 >( move ) );

            Move( nMove * moveBy, false );
        }
    }
    else
    {
		if (m_targetMovePosition == glm::ivec2())
		{
			m_currentNodeIdx = Game::GetInstance().GetLevel().GetPathfinder().GetNearestNode(m_centeredLocalPosition);
		}

        m_targetMovePosition = Game::GetInstance( ).GetLevel( ).GetPathfinder( ).GetNearestPosition( m_currentNodeIdx, playerPos );

        if( m_targetMovePosition != glm::ivec2( 0, 0 ) )
        {
            auto move  = m_targetMovePosition - m_centeredLocalPosition;
            auto nMove = glm::normalize( static_cast< glm::vec2 >( move ) );

            Move( nMove * moveBy, false );
        }
    }

    m_isAtInitialPos = false;
}

void Enemy::ReturnToInitialPosition( )
{
    Game::GetInstance( ).RenderText( "RETURNING", glm::vec2( 128.0f, 256.0f ), 1.0f, glm::vec3( 0.0f, 0.1f, 0.4f ) );

    auto moveBy             = m_movementSpeed * Game::GetInstance( ).GetDeltaTime( );
    auto vectorToInitialPos = static_cast< glm::vec2 >( m_initialPosition - m_centeredLocalPosition );
    auto lengthToInitialPos = glm::length( vectorToInitialPos );
    auto distanceToNode     = glm::length( static_cast< glm::vec2 >( m_targetMovePosition - m_centeredLocalPosition ) );

    if( lengthToInitialPos < 5.0f )
    {
        Move( vectorToInitialPos, false );
        m_currentNodeIdx = Game::GetInstance( ).GetLevel( ).GetPathfinder( ).FindNodeIdx( m_centeredLocalPosition );
        m_isAtInitialPos = true;
        return;
    }
    else if( ( distanceToNode < 5.0f ) && ( distanceToNode != 0.0f ) )
    {
        Move( m_targetMovePosition - m_centeredLocalPosition, false );
        m_currentNodeIdx = Game::GetInstance( ).GetLevel( ).GetPathfinder( ).FindNodeIdx( m_centeredLocalPosition );
    }
    else if( m_targetMovePosition == m_centeredLocalPosition )
    {
        m_currentNodeIdx     = Game::GetInstance( ).GetLevel( ).GetPathfinder( ).FindNodeIdx( m_centeredLocalPosition );
        m_targetMovePosition = Game::GetInstance( ).GetLevel( ).GetPathfinder( ).GetNearestPosition( /*m_centeredLocalPosition*/ m_currentNodeIdx, m_initialPosition );
        auto move            = m_targetMovePosition - m_centeredLocalPosition;
        auto nMove           = glm::normalize( static_cast< glm::vec2 >( move ) );

        Move( nMove * moveBy, false );
    }
    else
    {
        m_targetMovePosition = Game::GetInstance( ).GetLevel( ).GetPathfinder( ).GetNearestPosition( /*m_centeredLocalPosition*/ m_currentNodeIdx, m_initialPosition );

		if (m_targetMovePosition != glm::ivec2(0, 0))
		{
			auto move = m_targetMovePosition - m_centeredLocalPosition;
			auto nMove = glm::normalize(static_cast<glm::vec2>(move));

			Move(nMove * moveBy, false);
		}
    }
}

void Enemy::ClearPositions( )
{
    m_targetShootPosition    = glm::vec2( 0.0f, 0.0f );
    m_combatStarted          = false;
    m_timeSinceCombatStarted = 0.0f;
    m_timeSinceLastShot      = 0.0f;
}

void Enemy::Render( const Shaders& program )
{
    if( !m_combatStarted && m_isAtInitialPos )
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
