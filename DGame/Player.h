#pragma once
#include"Common.h"
#include"Shaders.h"
#include"Sprite.h"
#include"GameObject.h"
#include"Weapon.h"

class Player {
	std::string name;
	glm::vec2   localPosition;
	glm::vec2   globalPosition;
	Sprite      sprite;
	Shaders     program;
	Weapon* weapons;
	Weapon currentWeapon;
	
	GLuint vertexArrayBuffer;
	GLuint vertexBuffer;

	float angle;
	glm::mat4 translateMatrix;
	glm::vec2 translateVal;
	glm::mat4 rotateMatrix;
	glm::mat4 scaleMatrix;
public:
	Player(const glm::vec2& position = glm::vec2(0.0f, 0.0f),
		   const std::string& name = "Anonymous");
	~Player(); 
	
	void LoadShaders(const std::string& shaderFile);
	void LoadShaders(const Shaders& program);
	glm::vec2 GetGlobalPosition() const;
	glm::vec2 GetScreenPosition() const;
	glm::ivec2 GetScreenPositionPixels() const;
	void Move(const glm::vec2& vector);
	void CreateSprite(const glm::vec2& position = glm::vec2(0.0f, 0.0f), 
					  glm::ivec2 size = glm::ivec2(32,32), 
					  const std::string& fileName = ".\\Default.png");
	void SetGlobalPosition(const glm::vec2& position) { this->globalPosition = position; }// translateVal = position;

	void Draw();
	void Shoot();
	
};

