#pragma once
#include "Common.h"
#include"Shaders.h"

class Framebuffer {
	GLuint frameBufferID;
	GLuint textureID;
	Shaders shaders;
public:
	Framebuffer();
	~Framebuffer();

	void LoadShaders(const std::string& shaderName);
	void BeginDrawingToTexture();
	void EndDrawingToTexture();
};

