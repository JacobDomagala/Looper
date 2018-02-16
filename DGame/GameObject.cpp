#include "GameObject.h"
#include "Win_Window.h"
//#include "Level.h"
#include "Game.h"

GameObject::GameObject( const glm::vec2& pos, glm::ivec2 size, const std::string& sprite )
    : m_globalPosition( pos )
    , m_localPosition( Game::GetInstance( ).GetLevel( ).GetLocalVec( glm::ivec2( pos.x, -pos.y ) ) )
    , m_visible( true )
{
    m_collision              = m_sprite.SetSpriteTextured( pos, size, sprite );
    m_centeredGlobalPosition = m_sprite.GetCenteredPosition( );
    m_centeredLocalPosition  = Game::GetInstance( ).GetLevel( ).GetLocalVec( m_centeredGlobalPosition );
}

glm::vec2 GameObject::GetScreenPositionPixels( ) const
{
    //Get the world coords
    glm::vec4 screenPosition = Win_Window::GetInstance( ).GetProjection( ) * glm::vec4( m_centeredGlobalPosition, 0.0f, 1.0f );

    //convert from <-1,1> to <0,1>
    glm::vec2 tmpPos = ( glm::vec2( screenPosition.x, screenPosition.y ) + glm::vec2( 1.0f, 1.0f ) ) / glm::vec2( 2.0f, 2.0f );

    //convert from (0,0)->(1,1) [BOTTOM LEFT CORNER] to (0,0)->(WIDTH,HEIGHT) [TOP LEFT CORNER]
    tmpPos.x *= WIDTH;
    tmpPos.y *= -HEIGHT;
    tmpPos.y += HEIGHT;

    return tmpPos;
}

void GameObject::SetColor( const glm::vec3& color )
{
    m_sprite.SetColor( color );
}

glm::vec2 GameObject::GetSize( ) const
{
    return m_sprite.GetSize( );
}

glm::ivec2 GameObject::GetCenteredLocalPosition( ) const
{
    return m_centeredLocalPosition;
}

void GameObject::SetCenteredLocalPosition( glm::ivec2 pos )
{
    m_centeredLocalPosition = pos;
}

void GameObject::SetLocalPosition( const glm::ivec2& position )
{
    m_localPosition = position;
}

void GameObject::SetGlobalPosition( const glm::vec2& position )
{
    m_globalPosition = position;
}

glm::vec2 GameObject::GetGlobalPosition( ) const
{
    return m_globalPosition;
}

glm::ivec2 GameObject::GetLocalPosition( ) const
{
    return m_localPosition;
}

glm::vec2 GameObject::GetCenteredGlobalPosition( ) const
{
    return m_centeredGlobalPosition;
}

void GameObject::SetShaders( const Shaders& program )
{
    m_program = program;
}

void GameObject::SetTexture( Texture texture )
{
    m_sprite.SetTexture( texture );
}

void GameObject::CreateSprite( const glm::vec2& position, glm::ivec2 size )
{
    m_sprite.SetSprite( position, size );
    m_globalPosition = m_sprite.GetCenteredPosition( );
}

void GameObject::CreateSpriteTextured( const glm::vec2& position, glm::ivec2 size, const std::string& fileName )
{
    m_collision      = m_sprite.SetSpriteTextured( position, size, fileName );
    m_globalPosition = m_sprite.GetCenteredPosition( );
}

void GameObject::Move( const glm::vec2& moveBy, bool isCameraMovement )
{
    m_sprite.Translate( moveBy );
    m_globalPosition += moveBy;
    m_centeredGlobalPosition += moveBy;
    //if (!isCameraMovement)
    //{
    //	localPosition += moveBy;
    //	centeredLocalPosition += moveBy;
    //}
}

void GameObject::Render( const Shaders& program )
{
    m_sprite.Render( program );
}
