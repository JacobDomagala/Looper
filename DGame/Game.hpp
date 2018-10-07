#pragma once

#include "Common.hpp"
#include "Font.hpp"
#include "Framebuffer.hpp"
#include "Level.hpp"
#include "Player.hpp"
#include "Timer.hpp"
#include "Win_Window.hpp"

#pragma region DEBUG
/*
//
////PLZ DON'T MIND ME I'M JUST FOR DEBUG PURPOSES
//
*/

// THIS CLASS EXISTS ONLY IF WE WOULD NEED SOMETHING ELSE THAN LINE AS DEBUG OBJECT
class DebugObject
{
 public:
    virtual void Draw( )    = 0;
    virtual ~DebugObject( ) = default;
};

class Line : public DebugObject
{
    glm::vec2 m_from;
    glm::vec2 m_to;
    glm::vec3 m_color;

 public:
    Line( glm::vec2 from, glm::vec2 to, glm::vec3 color )
        : m_from( from )
        , m_to( to )
        , m_color( color )
    {
    }

    ~Line( ) override = default;

    void Draw( ) override
    {
        Shaders lineShader{};
        lineShader.LoadShaders( "lineVertex.glsl", "lineFragment.glsl" );

        glm::vec2 vertices[ 2 ] = {
            m_from,
            m_to
        };

        glm::mat4 modelMatrix = glm::scale( glm::mat4( ), glm::vec3( 1.0f, 1.0f, 1.0f ) );

        GLuint lineVertexArray;
        GLuint lineVertexBuffer;

        glGenVertexArrays( 1, &lineVertexArray );
        glGenBuffers( 1, &lineVertexBuffer );
        glBindVertexArray( lineVertexArray );
        glBindBuffer( GL_ARRAY_BUFFER, lineVertexBuffer );
        glBufferData( GL_ARRAY_BUFFER, sizeof( glm::vec2 ) * 2, vertices, GL_DYNAMIC_DRAW );
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( float ), nullptr );

        lineShader.UseProgram( );
        lineShader.SetUniformFloatMat4( modelMatrix, "modelMatrix" );
        lineShader.SetUniformFloatMat4( Win_Window::GetInstance( ).GetProjection( ), "projectionMatrix" );
        lineShader.SetUniformFloatVec4( glm::vec4( m_color, 1.0f ), "color" );

        glDrawArrays( GL_LINES, 0, 2 );
        glBindVertexArray( 0 );
        glDeleteBuffers( 1, &lineVertexBuffer );
        glDeleteVertexArrays( 1, &lineVertexArray );
    }
};
#pragma endregion

class Game
{
    //DEBUG
    std::vector< std::unique_ptr< DebugObject > > m_debugObjs;
    void                                          RenderLine( glm::ivec2 collided, glm::vec3 color );

    enum class GameState : char
    {
        MENU = 0,
        GAME,
        EDITOR
    };

    Timer m_timer;
    Level m_currentLevel;

    std::unique_ptr< byte_vec4 > m_collision;

    // active font used in game
    Font m_font;

    // how fast should camera move
    float m_cameraSpeed;

    // framebuffer for first pass
    Framebuffer m_frameBuffer;

    //
    float m_deltaTime;

    // all maps
    std::vector< std::string > m_levels;

    // state of the game
    GameState m_state;
    Player    m_player;

    // player position on map (centered)
    glm::vec2 m_playerPosition;

    // bullet collision for player
    glm::ivec2 CheckBulletCollision( int32_t range );
    glm::ivec2 CheckCollision( glm::ivec2& moveBy );
    glm::ivec2 CorrectPosition( );

    // convert from global position (OpenGL) to screen position (in pixels)
    glm::ivec2 GlobalToScreen( glm::vec2 globalPos );

    bool CheckMove( glm::ivec2& moveBy );
    void KeyEvents( float deltaTime );
    void MouseEvents( float deltaTime );

    // draws to framebuffer (texture)
    void RenderFirstPass( );

    // draws to screen
    void RenderSecondPass( );
    void LoadLevel( const std::string& levelName );

    void RayTracer( );

    Game( );

 public:
    // Singleton for Game class
    static Game& GetInstance( );

    // Initialize Game using 'configFile'
    void Init( std::string configFile );

    ~Game( ) = default;

    // Deleted copy constructor
    Game( Game& ) = delete;

    Player& GetPlayer( )
    {
        return m_player;
    }

    Level& GetLevel( )
    {
        return m_currentLevel;
    }

    float GetDeltaTime( ) const
    {
        return m_deltaTime;
    }

    std::pair< glm::ivec2, bool > CheckBulletCollision( Enemy* from, glm::vec2 targetPosition, int32_t range );
    bool                          IsPlayerInVision( Enemy* from, int32_t range );
    glm::ivec2                    CheckCollision( const glm::ivec2& currentPosition, const glm::ivec2& moveBy );

    void DrawLine( glm::vec2 from, glm::vec2 to, glm::vec3 color = glm::vec3( 1.0f, 0.0f, 0.0f ) );
    void RenderText( std::string text, const glm::vec2& position, float scale, const glm::vec3& color = glm::vec3( 1.0f, 1.0f, 1.0f ) );

    void ProcessInput( float deltaTime );
    void Render( );
};
