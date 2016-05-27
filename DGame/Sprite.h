#pragma once
#include"Common.h"
#include"Texture.h"
#include"Shaders.h"

class Sprite {
	Texture   texture;
	glm::vec4 color;
	glm::vec2 centeredPosition;
	glm::vec2 position;
	GLuint    vertexArrayBuffer;
	GLuint    vertexBuffer;

	glm::ivec2 size;

	glm::vec3 translateVal;
	glm::vec2 scaleVal;
	float     angle;
public:
	Sprite() { }
	~Sprite() { }

	void SetSprite(const glm::vec2& position = glm::vec2(0.0f, 0.0f), glm::ivec2 size = glm::ivec2(10, 10));
	charFour* SetSpriteTextured(const glm::vec2& position = glm::vec2(0.0f, 0.0f), glm::ivec2 size = glm::ivec2(10, 10), const std::string& fileName = ".\\Default.png");
	glm::vec2 GetCenteredPosition() const { return centeredPosition; }
	glm::vec2 GetPosition() const { return position; }
	glm::ivec2 GetSize() const { return size; }

	void SetColor(const glm::vec3& color) { this->color = glm::vec4(color, 1.0f); }
	
	void SetTextureFromFile(const std::string& filePath)
	{
		texture.LoadTextureFromFile(filePath);
	}
	void SetTexture(Texture& texture)
	{
		this->texture = texture;
	}

	void Rotate(float angle) { this->angle = angle; }
	void Scale(const glm::vec2& axies) { scaleVal = axies; }
	void Translate(const glm::vec2& axies) { 
		position += axies; 
		translateVal += glm::vec3(axies, 0.0f); }

	void Render(const Shaders& program);
};

