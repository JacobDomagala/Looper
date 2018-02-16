#pragma once

#include "Common.h"

class Texture
{
    // width and size of texture
    int m_width;
    int m_height;

    // texture ID used in
    GLuint m_textureID;

    // sampler ID
    GLuint m_samplerID;

    // each time new texture is loaded this counter is increased
    static int m_unitCounter;

    // ID of currently bound texture
    static int m_nowBound;

    // texture unit
    int m_unit;

 public:
    Texture( )  = default;
    ~Texture( ) = default;

    // DEBUG: number of glBindTexture calls
    static int m_boundCount;

    // Load texture from 'fileName' file and return byte values (used for collision)
    std::unique_ptr< byte_vec4 > LoadTextureFromFile( const std::string& fileName = "Assets//Default.png",
                                                      GLenum wrapMode = GL_REPEAT, GLenum filter = GL_LINEAR );

    // Load texture from 'data' memory
    void LoadTextureFromMemory( int width, int height, uint8* data,
                                GLenum wrapMode = GL_REPEAT, GLenum filter = GL_LINEAR );

    int GetWidth( ) const
    {
        return m_width;
    }

    int GetHeight( ) const
    {
        return m_height;
    }

    GLuint GetTextureHandle( ) const
    {
        return m_textureID;
    }

    // Make this texture active for given 'programID'
    void Use( GLuint programID );
};
