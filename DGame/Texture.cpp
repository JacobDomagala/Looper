#include "Texture.h"
#include "Win_Window.h"
#include "Shaders.h"

int Texture::m_unitCounter = 0;
int Texture::m_nowBound = 0;
int Texture::m_boundCount = 0;

std::unique_ptr<byte_vec4> Texture::LoadTextureFromFile(const std::string& fileName, GLenum wrapMode, GLenum filter)
{
	std::unique_ptr<byte_vec4> returnPtr(reinterpret_cast<byte_vec4*>(SOIL_load_image(fileName.c_str(), &m_width, &m_height, NULL, SOIL_LOAD_RGBA)));
	
	if (returnPtr == nullptr)
	{
		Win_Window::GetInstance()->ShowError(std::string("Can't load the file ") + fileName, "SOIL error!");
	}

	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_2D, m_textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<uint8*>(returnPtr.get()));
	glGenerateMipmap(GL_TEXTURE_2D);
	
	glSamplerParameteri(m_samplerID, GL_TEXTURE_MAG_FILTER, filter);
	glSamplerParameteri(m_samplerID, GL_TEXTURE_MIN_FILTER, filter);
	glSamplerParameteri(m_samplerID, GL_TEXTURE_WRAP_S, wrapMode);
	glSamplerParameteri(m_samplerID, GL_TEXTURE_WRAP_T, wrapMode);

	m_unit = m_unitCounter++;

	return std::move(returnPtr);
}

void Texture::LoadTextureFromMemory(int width, int height, uint8* data, GLenum wrapMode, GLenum filter)
{
	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_2D, m_textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glSamplerParameteri(m_samplerID, GL_TEXTURE_MAG_FILTER, filter);
	glSamplerParameteri(m_samplerID, GL_TEXTURE_MIN_FILTER, filter);
	glSamplerParameteri(m_samplerID, GL_TEXTURE_WRAP_S, wrapMode);
	glSamplerParameteri(m_samplerID, GL_TEXTURE_WRAP_T, wrapMode);

	m_unit = m_unitCounter++;
}

void Texture::Use(GLuint program)
{
	if (m_nowBound != m_unit)
	{
		GLuint samplerLocation = glGetUniformLocation(program, "texture");

		glActiveTexture(GL_TEXTURE0 + m_unit);
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glUniform1i(samplerLocation, m_unit);

		m_nowBound = m_unit;

		++m_boundCount;
	}
}


