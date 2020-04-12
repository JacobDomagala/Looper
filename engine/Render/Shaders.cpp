#include "Shaders.hpp"
#include "FileManager.hpp"

#include <GL/glew.h>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>

GLuint Shaders::m_activeProgramID = 0;
GLuint Shaders::m_numberBound = 0;

Shaders::~Shaders()
{
   glDeleteProgram(m_programID);
}

std::string
Shaders::GetName() const
{
   return m_name;
}

GLuint
Shaders::GetProgram() const
{
   return m_programID;
}

void
Shaders::UseProgram() const
{
   glUseProgram(m_programID);
   m_activeProgramID = m_programID;
   ++m_numberBound;
}

void
Shaders::LoadDefault()
{
   LoadShaders("DefaultShader");
}

std::string
Shaders::ReadShaderFile(std::string fileName)
{
   return FileManager::ReadFile((SHADERS_DIR / fileName).u8string());
}

void
Shaders::LoadShaders(const std::string& shaderName)
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

   m_logger.Log(Logger::TYPE::DEBUG, "Loaded shaders " + shaderName + " with program_id = " + std::to_string(m_programID));
}

void
Shaders::CheckCompileStatus(GLuint shaderID)
{
   GLint isCompleted = 0;
   glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompleted);
   if (isCompleted == GL_FALSE)
   {
      GLint maxLength = 0;
      glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

      std::string log(maxLength, 0);
      glGetShaderInfoLog(shaderID, maxLength, &maxLength, &log[0]);

      m_logger.Log(Logger::TYPE::FATAL, log);
   }
}

void
Shaders::CheckLinkStatus(GLuint programID)
{
   GLint isLinked = 0;
   glGetProgramiv(programID, GL_LINK_STATUS, &isLinked);
   if (isLinked == GL_FALSE)
   {
      GLint maxLength = 0;
      glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);

      std::string log(maxLength, 0);
      glGetProgramInfoLog(programID, maxLength, &maxLength, &log[0]);

      m_logger.Log(Logger::TYPE::FATAL, log);
   }
}

std::optional< GLint >
Shaders::GetUniformLocation(const std::string& uniformName)
{
   GLint location = glGetUniformLocation(m_programID, uniformName.c_str());
   if (!location)
   {
      m_logger.Log(Logger::TYPE::WARNING, "Uniform location not found. Uniform name: " + uniformName);
      return {};
   }

   return {location};
   //std::optional< GLint >(location);
}

void
Shaders::SetUniformBool(int value, const std::string& name)
{
   GLint location = glGetUniformLocation(m_programID, name.c_str());
   glUniform1i(location, value);
}

void
Shaders::SetUniformFloat(float value, const std::string& name)
{
   GLint location = glGetUniformLocation(m_programID, name.c_str());
   glUniform1f(location, value);
}

void
Shaders::SetUniformFloatVec2(const glm::vec2& value, const std::string& name)
{
   const auto location = GetUniformLocation(name);
   if (location.has_value())
   {
      glUniform2fv(location.value(), 1, glm::value_ptr(value));
   }
}

void
Shaders::SetUniformFloatVec4(const glm::vec4& value, const std::string& name)
{
   GLint location = glGetUniformLocation(m_programID, name.c_str());
   glUniform4fv(location, 1, glm::value_ptr(value));
}

void
Shaders::SetUniformFloatMat4(const glm::mat4& value, const std::string& name)
{
   GLint location = glGetUniformLocation(m_programID, name.c_str());
   glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}
