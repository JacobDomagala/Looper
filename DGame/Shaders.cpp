#include "Shaders.h"
#include"Win_Window.h"

GLuint Shaders::activeProgramID = 0;
GLuint Shaders::numberBound = 0;

GLuint Shaders::GetProgram() const
{
	return programID;
}

void Shaders::UseProgram() const
{
	if (programID != activeProgramID)
	{
		glUseProgram(programID);
		activeProgramID = programID;
		++numberBound;
	}
}

void Shaders::LoadDefault()
{
	LoadShaders("Shaders\\DefaultShader_vs.glsl", "Shaders\\DefaultShader_fs.glsl");
}

std::string Shaders::ReadShaderFile(const std::string& fileName)
{
	std::ifstream fileHandle;
	std::string shaderSource = "";
	fileHandle.open(fileName, std::ifstream::in);
	if (!fileHandle.is_open())
	{
		Win_Window::GetInstance()->ShowError(fileName + " can't be opened!", "Opening shader file");
	}
	
	while (!fileHandle.eof())
	{
		char tmp[1];
		fileHandle.read(tmp, 1);
		if(!fileHandle.eof())
			shaderSource += tmp[0];
	}
	fileHandle.close();
	
	return shaderSource;
}

void Shaders::LoadShaders(const std::string& vertexShader, const std::string& FragmentShader)
{
	vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	std::string tmp = ReadShaderFile(vertexShader);
	const GLchar* shaderSource = tmp.c_str();
	glShaderSource(vertexShaderID, 1, &shaderSource, NULL);

	tmp = ReadShaderFile(FragmentShader);
	shaderSource = tmp.c_str();
	glShaderSource(fragmentShaderID, 1, &shaderSource, NULL);

	glCompileShader(vertexShaderID);
	CheckCompileStatus(vertexShaderID);
	glCompileShader(fragmentShaderID);
	CheckCompileStatus(fragmentShaderID);

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);
	CheckLinkStatus(programID);

	glUseProgram(programID);

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);
}

void Shaders::CheckCompileStatus(GLuint shaderID)
{
	GLint isCompiled = 0;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

		char* log = new char[maxLength];
		glGetShaderInfoLog(shaderID, maxLength, &maxLength, &log[0]);
		Win_Window::GetInstance()->ShowError(log, "Compiling OpenGL program");
		glDeleteShader(shaderID); 

		delete[] (log);
	}
}
void Shaders::CheckLinkStatus(GLuint programID)
{
	GLint isLinked = 0;
	glGetProgramiv(programID, GL_LINK_STATUS, &isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);

		char* log = new char[maxLength];
		glGetProgramInfoLog(programID, maxLength, &maxLength, &log[0]);
		Win_Window::GetInstance()->ShowError(log, "Linking OpenGL program");

		glDeleteProgram(programID);

		delete[](log);
	}
}

void Shaders::SetUniformFloat(float value, const std::string& name) const
{
	GLint location = glGetUniformLocation(programID, name.c_str());
	glUniform1f(location, value);
}
void Shaders::SetUniformFloatVec2(const glm::vec2& value, const std::string& name) const
{
	GLint location = glGetUniformLocation(programID, name.c_str());
	glUniform2fv(location, 1, glm::value_ptr(value));
}
void Shaders::SetUniformFloatVec4(const glm::vec4& value, const std::string& name) const
{
	GLint location = glGetUniformLocation(programID, name.c_str());
	glUniform4fv(location, 1, glm::value_ptr(value));
}
void Shaders::SetUniformFloatMat4(const glm::mat4& value, const std::string& name) const
{
	GLint location = glGetUniformLocation(programID, name.c_str());
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}
