#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

class Shaders
{
   GLuint m_programID;
   static GLuint m_activeProgramID;

   std::string
   ReadShaderFile(std::string fileName);

   void
   CheckCompileStatus(GLuint shaderID);
   void
   CheckLinkStatus(GLuint programID);

 public:
   static GLuint m_numberBound;

   Shaders() = default;
   ~Shaders()
   {
      glDeleteProgram(m_programID);
   }

   GLuint
   GetProgram() const;
   void
   UseProgram() const;

   void
   LoadDefault();
   void
   LoadShaders(const std::string& vertexShader, const std::string& FragmentShader);

   void
   SetUniformFloat(float value, const std::string& name) const;
   void
   SetUniformFloatVec2(const glm::vec2& value, const std::string& name) const;
   void
   SetUniformFloatVec4(const glm::vec4& value, const std::string& name) const;
   void
   SetUniformFloatMat4(const glm::mat4& value, const std::string& name) const;
};
