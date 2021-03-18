#pragma once

#include "Logger.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

namespace dgame {

class Shader
{
 public:
   Shader(const std::string& shaderName);
   Shader() = default;
   ~Shader();

   std::string
   GetName() const;

   GLuint
   GetProgram() const;

   void
   UseProgram() const;

   void
   LoadDefault();

   void
   LoadShaders(const std::string& shaderName);

   void
   SetUniformBool(bool value, const std::string& name);

   void
   SetUniformFloat(float value, const std::string& name);

   void
   SetUniformFloatVec2(const glm::vec2& value, const std::string& name);

   void
   SetUniformFloatVec4(const glm::vec4& value, const std::string& name);

   void
   SetUniformFloatMat4(const glm::mat4& value, const std::string& name);

   void
   SetUniformIntArray(const int* value, int count, const std::string& name);

 private:
   std::string
   ReadShaderFile(std::string fileName);

   void
   CheckCompileStatus(GLuint shaderID);

   void
   CheckLinkStatus(GLuint programID);

   GLint
   GetUniformLocation(const std::string& uniformName);

   std::string m_name;
   GLuint m_programID;

   Logger m_logger = Logger("Shader");
};

} // namespace dgame
