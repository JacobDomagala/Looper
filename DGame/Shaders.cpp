#include <Shaders.hpp>
#include <Win_Window.hpp>
#include <glew.h>
#include <fstream>
#include <gtc/type_ptr.hpp>

GLuint Shaders::m_activeProgramID = 0;
GLuint Shaders::m_numberBound     = 0;

GLuint Shaders::GetProgram( ) const
{
    return m_programID;
}

void Shaders::UseProgram( ) const
{
    if(m_programID != m_activeProgramID)
    {
        glUseProgram(m_programID);
		m_activeProgramID = m_programID;
        ++m_numberBound;
    }
}

void Shaders::LoadDefault( )
{
    LoadShaders( "../Shaders\\DefaultShader_vs.glsl", "../Shaders\\DefaultShader_fs.glsl" );
}

std::string Shaders::ReadShaderFile( const std::string& fileName )
{
    std::ifstream fileHandle;
    std::string   shaderSource = "";
    fileHandle.open( fileName, std::ifstream::in );
    if( !fileHandle.is_open( ) )
    {
        Win_Window::GetInstance( ).ShowError( fileName + " can't be opened!", "Opening shader file" );
    }

    while( !fileHandle.eof( ) )
    {
        char tmp[ 1 ];
        fileHandle.read( tmp, 1 );
        if( !fileHandle.eof( ) )
            shaderSource += tmp[ 0 ];
    }
    fileHandle.close( );

    return shaderSource;
}

void Shaders::LoadShaders( const std::string& vertexShader, const std::string& FragmentShader )
{
    m_vertexShaderID   = glCreateShader( GL_VERTEX_SHADER );
    m_fragmentShaderID = glCreateShader( GL_FRAGMENT_SHADER );

    std::string   tmp          = ReadShaderFile( vertexShader );
    const GLchar* shaderSource = tmp.c_str( );
    glShaderSource(m_vertexShaderID, 1, &shaderSource, NULL );

    tmp          = ReadShaderFile( FragmentShader );
    shaderSource = tmp.c_str( );
    glShaderSource(m_fragmentShaderID, 1, &shaderSource, NULL );

    glCompileShader(m_vertexShaderID);
    CheckCompileStatus(m_vertexShaderID);
    glCompileShader(m_fragmentShaderID);
    CheckCompileStatus(m_fragmentShaderID);

    m_programID = glCreateProgram( );
    glAttachShader(m_programID, m_vertexShaderID);
    glAttachShader(m_programID, m_fragmentShaderID);
    glLinkProgram(m_programID);
    CheckLinkStatus(m_programID);

    glUseProgram(m_programID);

    glDeleteShader(m_vertexShaderID);
    glDeleteShader(m_fragmentShaderID);
}

void Shaders::CheckCompileStatus( GLuint shaderID )
{
    GLint isCompiled = 0;
    glGetShaderiv( shaderID, GL_COMPILE_STATUS, &isCompiled );
    if( isCompiled == GL_FALSE )
    {
        GLint maxLength = 0;
        glGetShaderiv( shaderID, GL_INFO_LOG_LENGTH, &maxLength );

        char* log = new char[ maxLength ];
        glGetShaderInfoLog( shaderID, maxLength, &maxLength, &log[ 0 ] );
        Win_Window::GetInstance( ).ShowError( log, "Compiling OpenGL program" );
        glDeleteShader( shaderID );

        delete[]( log );
    }
}

void Shaders::CheckLinkStatus( GLuint programID )
{
    GLint isLinked = 0;
    glGetProgramiv( programID, GL_LINK_STATUS, &isLinked );
    if( isLinked == GL_FALSE )
    {
        GLint maxLength = 0;
        glGetProgramiv( programID, GL_INFO_LOG_LENGTH, &maxLength );

        char* log = new char[ maxLength ];
        glGetProgramInfoLog( programID, maxLength, &maxLength, &log[ 0 ] );
        Win_Window::GetInstance( ).ShowError( log, "Linking OpenGL program" );

        glDeleteProgram( programID );

        delete[]( log );
    }
}

void Shaders::SetUniformFloat( float value, const std::string& name ) const
{
    GLint location = glGetUniformLocation( m_programID, name.c_str( ) );
    glUniform1f( location, value );
}

void Shaders::SetUniformFloatVec2( const glm::vec2& value, const std::string& name ) const
{
    GLint location = glGetUniformLocation(m_programID, name.c_str( ) );
    glUniform2fv( location, 1, glm::value_ptr( value ) );
}

void Shaders::SetUniformFloatVec4( const glm::vec4& value, const std::string& name ) const
{
    GLint location = glGetUniformLocation(m_programID, name.c_str( ) );
    glUniform4fv( location, 1, glm::value_ptr( value ) );
}

void Shaders::SetUniformFloatMat4( const glm::mat4& value, const std::string& name ) const
{
    GLint location = glGetUniformLocation(m_programID, name.c_str( ) );
    glUniformMatrix4fv( location, 1, GL_FALSE, glm::value_ptr( value ) );
}
