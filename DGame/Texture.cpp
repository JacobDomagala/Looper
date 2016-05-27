#include "Texture.h"
#include"Win_Window.h"
#include"Shaders.h"
extern Win_Window* window;

int Texture::unitCounter = 0;
int Texture::nowBound = 0;
int Texture::boundCount = 0;

charFour* Texture::LoadTextureFromFile(const std::string& fileName, GLenum wrapMode, GLenum filter)
{
	//NOTE: WE'RE NOT FREEING THE POINTER MEMORY!!!
	charFour* returnPtr;
	fileData = SOIL_load_image(fileName.c_str(), &width, &height, NULL, SOIL_LOAD_RGBA);
	if (!fileData)
		window->ShowError(std::string("Can't load the file ") + fileName, "SOIL error!");

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, fileData);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	glSamplerParameteri(samplerID, GL_TEXTURE_MAG_FILTER, filter);
	glSamplerParameteri(samplerID, GL_TEXTURE_MIN_FILTER, filter);
	glSamplerParameteri(samplerID, GL_TEXTURE_WRAP_S, wrapMode);
	glSamplerParameteri(samplerID, GL_TEXTURE_WRAP_T, wrapMode);

	returnPtr = (charFour*)fileData;
	//SOIL_free_image_data(fileData);
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


