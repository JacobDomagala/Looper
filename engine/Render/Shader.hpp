#pragma once

#include "Logger.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

namespace looper {

// NOLINTNEXTLINE
class Shader
{
 public:
   explicit Shader(const std::string& shaderName);
   Shader() = default;
   ~Shader();

   [[nodiscard]] std::string
   GetName() const;

   [[nodiscard]] GLuint
   GetProgram() const;

   void
   UseProgram() const;

   void
   LoadDefault();

   void
   LoadShaders(const std::string& shaderName);

   void
   SetUniformBool(bool value, const std::string& name) const;

   void
   SetUniformFloat(float value, const std::string& name) const;

   void
   SetUniformFloatVec2(const glm::vec2& value, const std::string& name);

   void
   SetUniformFloatVec4(const glm::vec4& value, const std::string& name) const;

   void
   SetUniformFloatMat4(const glm::mat4& value, const std::string& name) const;

   void
   SetUniformIntArray(const int* value, int count, const std::string& name) const;

 private:
   std::string
   ReadShaderFile(const std::string& fileName);

   void
   CheckCompileStatus(GLuint shaderID) const;

   void
   CheckLinkStatus(GLuint programID) const;

   GLint
   GetUniformLocation(const std::string& uniformName);

   std::string m_name = {};
   GLuint m_programID = {};
};

} // namespace looper
