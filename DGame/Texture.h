#pragma once

#include "Common.h"

class Texture 
{
	int width, height;
	GLuint textureID;
	GLuint samplerID;
	uint8* fileData;
	static int unitCounter;
	static int nowBound;
	int unit;

public:
	Texture() = default;
	~Texture() = default;
	
	static int boundCount;

	std::shared_ptr<byte_vec4> LoadTextureFromFile(const std::string& fileName = "Assets//Default.png", 
							 GLenum wrapMode = GL_REPEAT, GLenum filter = GL_LINEAR);
	void LoadTextureFromMemory(int width, int height, uint8* data, 
							   GLenum wrapMode = GL_REPEAT, GLenum filter = GL_LINEAR);
	int GetWidth() const { return width; }
	int GetHeight() const { return height; }
	GLuint GetTextureHandle() const { return textureID; }

	void Use(GLuint programID);
};

