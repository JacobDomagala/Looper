#pragma once
#include"Common.h"
#include"Sprite.h"
#include"Shaders.h"
#include"Timer.h"


class GameObject {
protected:
	glm::vec2 position;
	Sprite sprite;
	Shaders program;
	Timer tmpTimer;

	glm::mat4 translateMatrix;
	glm::vec2 translateVal;
	glm::mat4 rotateMatrix;
	glm::mat4 scaleMatrix;
public:
	GameObject() {}
	~GameObject() {}

	void SetPosition(const glm::vec2& position)
	{
		this->position = position; 
	}
	glm::vec2 GetPosition() 
	{
		return position; 
	}
	void SetShaders(const Shaders& program) 
	{
		this->program = program; 
	}
	void SetTexture(Texture texture)
	{
		sprite.SetTexture(texture);
	}
	virtual void CreateSprite(const glm::vec2& position = glm::vec2(0.0f, 0.0f), glm::ivec2 size = glm::ivec2(10, 10))
	{
		sprite.SetSprite(position, size);
		this->position = sprite.GetCenteredPosition();
	}
	virtual void CreateSpriteTextured(const glm::vec2& position = glm::vec2(0.0f, 0.0f), glm::ivec2 size = glm::ivec2(16, 16), const std::string& fileName = ".\\Default.png")
	{
		sprite.SetSpriteTextured(position, size, fileName);
		this->position = sprite.GetCenteredPosition();
	}
	virtual void Move(const glm::vec2& moveBy)
	{
		sprite.Translate(moveBy);
		position += moveBy;
	}
	virtual void Render(const Shaders& program) 
	{
		sprite.Render(program); 
	}
};

