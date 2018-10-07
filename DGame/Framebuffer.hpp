#pragma once

#include "Common.hpp"
#include "Shaders.hpp"

class Framebuffer
{
    GLuint  frameBufferID;
    GLuint  textureID;
    Shaders shaders;

 public:
    Framebuffer( );
    ~Framebuffer( ) = default;

    void LoadShaders( const std::string& shaderName );
    void BeginDrawingToTexture( );
    void EndDrawingToTexture( );
};
