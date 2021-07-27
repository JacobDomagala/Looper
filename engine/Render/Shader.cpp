#include "Shader.hpp"
#include "FileManager.hpp"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
namespace dgame {

Shader::Shader(const std::string& shaderName)
{
   LoadShaders(shaderName);
}

Shader::~Shader()
{
   glDeleteProgram(m_programID);
}

std::string
Shader::GetName() const
{
   return m_name;
}

GLuint
Shader::GetProgram() const
{
   return m_programID;
}

void
Shader::UseProgram() const
{
   m_logger.Log(Logger::TYPE::TRACE, "Binding Shader program {}", m_programID);
   glUseProgram(m_programID);
}

void
Shader::LoadDefault()
{
   LoadShaders("DefaultShader");
}

std::string
Shader::ReadShaderFile(std::string fileName)
{
   return FileManager::ReadFile((SHADERS_DIR / fileName).string());
}

void
Shader::LoadShaders(const std::string& shaderName)
{
   m_name = shaderName;

   auto vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
   auto fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

   std::string tmp = ReadShaderFile(shaderName + "_vs.glsl");
   const GLchar* shaderSource = tmp.c_str();
   glShaderSource(vertexShaderID, 1, &shaderSource, NULL);

   tmp = ReadShaderFile(shaderName + "_fs.glsl");
   shaderSource = tmp.c_str();
   glShaderSource(fragmentShaderID, 1, &shaderSource, NULL);

   glCompileShader(vertexShaderID);
   CheckCompileStatus(vertexShaderID);
   glCompileShader(fragmentShaderID);
   CheckCompileStatus(fragmentShaderID);

   m_programID = glCreateProgram();
   glAttachShader(m_programID, vertexShaderID);
   glAttachShader(m_programID, fragmentShaderID);
   glLinkProgram(m_programID);
   CheckLinkStatus(m_programID);

   glUseProgram(m_programID);

   glDeleteShader(vertexShaderID);
   glDeleteShader(fragmentShaderID);

   m_logger.Log(Logger::TYPE::DEBUG, "Loaded shader {} bound to ID = {}", shaderName, m_programID);
}

void
Shader::CheckCompileStatus(GLuint shaderID)
{
   GLint isCompleted = 0;
   glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompleted);
   if (isCompleted == GL_FALSE)
   {
      GLint maxLength = 0;
      glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

      std::string log(static_cast< std::string::size_type >(maxLength), '\0');
      glGetShaderInfoLog(shaderID, maxLength, &maxLength, &log[0]);

      m_logger.Log(Logger::TYPE::FATAL, log);
   }
}

void
Shader::CheckLinkStatus(GLuint programID)
{
   GLint isLinked = 0;
   glGetProgramiv(programID, GL_LINK_STATUS, &isLinked);
   if (isLinked == GL_FALSE)
   {
      GLint maxLength = 0;
      glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);

      std::string log(static_cast< std::string::size_type >(maxLength), '\0');
      glGetProgramInfoLog(programID, maxLength, &maxLength, &log[0]);

      m_logger.Log(Logger::TYPE::FATAL, log);
   }
}

GLint
Shader::GetUniformLocation(const std::string& uniformName)
{
   GLint location = glGetUniformLocation(m_programID, uniformName.c_str());
   if (!location)
   {
      m_logger.Log(Logger::TYPE::WARNING, "Uniform location not found. Uniform name: {}",
                   uniformName);
   }

   return location;
}

void
Shader::SetUniformBool(bool value, const std::string& name)
{
   GLint location = glGetUniformLocation(m_programID, name.c_str());
   glUniform1i(location, value);
}

void
Shader::SetUniformFloat(float value, const std::string& name)
{
   GLint location = glGetUniformLocation(m_programID, name.c_str());
   glUniform1f(location, value);
}

void
Shader::SetUniformFloatVec2(const glm::vec2& value, const std::string& name)
{
   const auto location = GetUniformLocation(name);
   if (location)
   {
      glUniform2fv(location, 1, glm::value_ptr(value));
   }
}

void
Shader::SetUniformFloatVec4(const glm::vec4& value, const std::string& name)
{
   GLint location = glGetUniformLocation(m_programID, name.c_str());
   glUniform4fv(location, 1, glm::value_ptr(value));
}

void
Shader::SetUniformFloatMat4(const glm::mat4& value, const std::string& name)
{
   GLint location = glGetUniformLocation(m_programID, name.c_str());
   glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void
Shader::SetUniformIntArray(const int* value, int count, const std::string& name)
{
   GLint location = glGetUniformLocation(m_programID, name.c_str());
   glUniform1iv(location, count, value);
}

} // namespace dgame
