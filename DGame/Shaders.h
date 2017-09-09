#pragma once

#include "Common.h"

enum class ShaderType : uint8
{
	VERTEX_SHADER,
	FRAGMENT_SHADER
};

class Shaders 
{
	GLuint programID;
	GLuint vertexShaderID;
	GLuint fragmentShaderID;
	static GLuint activeProgramID;
	
	std::string ReadShaderFile(const std::string& fileName);
	
	void CheckCompileStatus(GLuint shaderID);
	void CheckLinkStatus(GLuint programID);

public:
	static GLuint numberBound;

	Shaders() = default;
	~Shaders() 
	{
		glDeleteProgram(programID);
	}
	
	GLuint GetProgram() const;
	void UseProgram() const;
	
	void LoadDefault();
	void LoadShaders(const std::string& vertexShader, const std::string& FragmentShader);

	void SetUniformFloat(float value, const std::string& name)const;
	void SetUniformFloatVec2(const glm::vec2& value, const std::string& name)const;
	void SetUniformFloatVec4(const glm::vec4& value, const std::string& name)const;
	void SetUniformFloatMat4(const glm::mat4& value, const std::string& name)const;
	
};

