#pragma once

#include "Common.h"
#include "Shaders.h"
#include "Texture.h"

class Sprite
{
    // sprite's texture
    Texture m_texture{};

    // color of sprite (default is white)
    glm::vec4 m_color;

    // sprite's center
    glm::vec2 m_centeredPosition;

    // sprite's position
    glm::vec2 m_position;

    // OpenGL buffers
    GLuint m_vertexArrayBuffer{};
    GLuint m_vertexBuffer{};

    // width and height
    glm::ivec2 m_size;

    // transofmation values
    glm::vec3 m_translateVal;
    glm::vec3 m_velocity;
    glm::vec2 m_scaleVal;
    float     m_angle{};

 public:
    Sprite( )  = default;
    ~Sprite( ) = default;

    // Create sprite without texture
    void SetSprite( const glm::vec2& position = glm::vec2( 0.0f, 0.0f ), glm::ivec2 size = glm::ivec2( 10, 10 ) );

    // Create sprite with texture
    // Returns byte vector used for collision
    std::unique_ptr< byte_vec4 > SetSpriteTextured( const glm::vec2& position = glm::vec2( 0.0f, 0.0f ), glm::ivec2 size = glm::ivec2( 10, 10 ), const std::string& fileName = ".\\Default.png" );

    glm::vec2 GetCenteredPosition( ) const
    {
        return m_centeredPosition;
    }

    glm::vec2 GetPosition( ) const
    {
        return m_position;
    }

    glm::ivec2 GetSize( ) const
    {
        return m_size;
    }

    void SetColor( const glm::vec3& color )
    {
        m_color = glm::vec4( color, 1.0f );
    }

    void SetTextureFromFile( const std::string& filePath )
    {
        m_texture.LoadTextureFromFile( filePath );
    }

    void SetTexture( Texture& texture )
    {
        m_texture = texture;
    }

    void Rotate( float angle )
    {
        m_angle = angle;
    }

    void Scale( const glm::vec2& axies )
    {
        m_scaleVal = axies;
    }

    void Translate( const glm::vec2& axies )
    {
        m_position += axies;
        m_translateVal += glm::vec3( axies, 0.0f );
    }

    // Render sprite using 'program'
    void Render( const Shaders& program );
};
