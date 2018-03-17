#pragma once

#include "Common.h"

class Shaders
{
    enum class ShaderType : uint8
    {
        VERTEX_SHADER,
        FRAGMENT_SHADER
    };

    GLuint        m_programID;
    GLuint        m_vertexShaderID;
    GLuint        m_fragmentShaderID;
    static GLuint m_activeProgramID;

    std::string ReadShaderFile( const std::string& fileName );

    void CheckCompileStatus( GLuint shaderID );
    void CheckLinkStatus( GLuint programID );

 public:
    static GLuint m_numberBound;

    Shaders( ) = default;
    ~Shaders( )
    {
        glDeleteProgram( m_programID );
    }

    GLuint GetProgram( ) const;
    void   UseProgram( ) const;

    void LoadDefault( );
    void LoadShaders( const std::string& vertexShader, const std::string& FragmentShader );

    void SetUniformFloat( float value, const std::string& name ) const;
    void SetUniformFloatVec2( const glm::vec2& value, const std::string& name ) const;
    void SetUniformFloatVec4( const glm::vec4& value, const std::string& name ) const;
    void SetUniformFloatMat4( const glm::mat4& value, const std::string& name ) const;
};
