#include "Texture.h"
#include "Win_Window.h"
#include "Shaders.h"

int Texture::unitCounter = 0;
int Texture::nowBound = 0;
int Texture::boundCount = 0;

std::shared_ptr<byte_vec4> Texture::LoadTextureFromFile(const std::string& fileName, GLenum wrapMode, GLenum filter)
{
	std::shared_ptr<byte_vec4> returnPtr(reinterpret_cast<byte_vec4*>(SOIL_load_image(fileName.c_str(), &width, &height, NULL, SOIL_LOAD_RGBA)));
	
	if (!returnPtr)
		Win_Window::GetInstance()->ShowError(std::string("Can't load the file ") + fileName, "SOIL error!");

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<uint8*>(returnPtr.get()));
	glGenerateMipmap(GL_TEXTURE_2D);
	
	glSamplerParameteri(samplerID, GL_TEXTURE_MAG_FILTER, filter);
	glSamplerParameteri(samplerID, GL_TEXTURE_MIN_FILTER, filter);
	glSamplerParameteri(samplerID, GL_TEXTURE_WRAP_S, wrapMode);
	glSamplerParameteri(samplerID, GL_TEXTURE_WRAP_T, wrapMode);

	unit = unitCounter++;

	return returnPtr;
}

void Texture::LoadTextureFromMemory(int width, int height, uint8* data, GLenum wrapMode, GLenum filter)
{
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glSamplerParameteri(samplerID, GL_TEXTURE_MAG_FILTER, filter);
	glSamplerParameteri(samplerID, GL_TEXTURE_MIN_FILTER, filter);
	glSamplerParameteri(samplerID, GL_TEXTURE_WRAP_S, wrapMode);
	glSamplerParameteri(samplerID, GL_TEXTURE_WRAP_T, wrapMode);
	unit = unitCounter++;
}

void Texture::Use(GLuint program)
{
	if (nowBound != unit)
	{
		GLuint samplerLocation = glGetUniformLocation(program, "texture");
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(samplerLocation, unit);
		nowBound = unit;
		++boundCount;
	}
}


