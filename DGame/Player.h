#pragma once
#include"Common.h"
#include"Shaders.h"
#include"Sprite.h"
#include"GameObject.h"

class Player {
	std::string name;
	glm::vec2   position;
	Sprite      sprite;
	Shaders     program;


	
	
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
	void Move(const glm::vec2& vector);
	void CreateSprite(const glm::vec2& position = glm::vec2(0.0f, 0.0f), 
					  glm::ivec2 size = glm::ivec2(32,32), 
					  const std::string& fileName = ".\\Default.png");
	void SetPosition(const glm::vec2& position) { this->position = position; }// translateVal = position;

	void Draw();
	void Shoot();
	
};

