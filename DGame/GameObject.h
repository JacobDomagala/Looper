#pragma once
#include"Common.h"
#include"Sprite.h"
#include"Shaders.h"


class GameObject {
protected:
	
	glm::vec2 globalPosition;
	glm::vec2 centeredGlobalPosition;
	
	glm::ivec2 localPosition;
	glm::ivec2 centeredLocalPosition;

	Sprite sprite;
	Shaders program;

	bool drawMe;
	charFour* collision;

	glm::mat4 translateMatrix;
	glm::vec2 translateVal;
	glm::mat4 rotateMatrix;
	glm::mat4 scaleMatrix;
public:
	virtual void Shoot() {}
	virtual void SetPlayerPos(glm::vec2) {}
	virtual void Hit(int dmg) {}
	virtual bool GetState() { return drawMe; }
	GameObject() {}
	GameObject(const glm::vec2& pos, glm::ivec2 size, const std::string& sprite);
	~GameObject() {}
	bool CheckCollision(const glm::ivec2& position) const;
	void SetColor(const glm::vec3& color)
	{
		sprite.SetColor(color);
	}
	glm::vec2 GetSize()
	{
		return sprite.GetSize();
	}
	glm::ivec2 GetCenteredLocalPosition()
	{
		return centeredLocalPosition;
	}
	void SetCenteredLocalPosition(glm::ivec2 pos)
	{
		centeredLocalPosition = pos;
	}
	void SetLocalPosition(const glm::ivec2& position)
	{
		localPosition = position;
	}
	void SetGlobalPosition(const glm::vec2& position)
	{
		globalPosition = position;
	}
	inline glm::vec2 GetGlobalPosition() 
	{
		return globalPosition;
	}
	glm::ivec2 GetLocalPosition()
	{
		return localPosition;
	}
	glm::vec2 GetCenteredGlobalPosition()
	{
		return centeredGlobalPosition;
	}
	glm::vec2 GetScreenPositionPixels();
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
		globalPosition = sprite.GetCenteredPosition();
	}
	virtual void CreateSpriteTextured(const glm::vec2& position = glm::vec2(0.0f, 0.0f), glm::ivec2 size = glm::ivec2(16, 16), const std::string& fileName = ".\\Default.png")
	{
		collision = sprite.SetSpriteTextured(position, size, fileName);
		globalPosition = sprite.GetCenteredPosition();
	}
	virtual void Move(const glm::vec2& moveBy, bool isCameraMovement = true)
	{
		sprite.Translate(moveBy);
		globalPosition += moveBy;
		centeredGlobalPosition += moveBy;
		//if (!isCameraMovement)
		//{
		//	localPosition += moveBy;
		//	centeredLocalPosition += moveBy;
		//}
		
	}
	virtual void Render(const Shaders& program) 
	{
		sprite.Render(program); 
	}
};

