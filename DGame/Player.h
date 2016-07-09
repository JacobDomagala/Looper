#pragma once
#include"Common.h"
#include"Shaders.h"
#include"Sprite.h"
#include"GameObject.h"
#include"Weapon.h"
class Enemy;

class Player {

	// name of the player
	std::string name;

	// local position with origin in upper left corner
	glm::ivec2 localPosition;

	// local position with origin in the center of sprite
	glm::ivec2 centeredLocalPosition;

	// global position in the origin in the upper left corner
	glm::vec2  globalPosition;

	// global position in the origin in the center of sprite
	glm::vec2  centeredGlobalPosition;

	Sprite      sprite;
	Shaders     program;

	Weapon* weapons[3];
	Weapon* currentWeapon;

	int maxHP;
	int currentHP;
	
	//leaking memory, GET RID OF IT
	charFour* collision;
	GLuint vertexArrayBuffer;
	GLuint vertexBuffer;

	float angle;
	//glm::mat4 translateMatrix;
	glm::vec2 translateVal;
	//glm::mat4 rotateMatrix;
	//glm::mat4 scaleMatrix;
public:
	Player(glm::vec2 position = glm::vec2(0.0f, 0.0f),
		   const std::string& name = "Anonymous");
	~Player(); 

	bool CheckCollision(glm::ivec2 pos, GameObject* obj);

	void LoadShaders(const std::string& shaderFile);
	void LoadShaders(const Shaders& program);

	void SetLocalPosition(glm::ivec2 pos); 
	void SetCenteredLocalPosition(glm::ivec2 pos);
	void SetGlobalPosition(glm::vec2 position);
	glm::vec2 GetGlobalPosition() const;
	glm::vec2 GetCenteredGlobalPosition() const;
	glm::vec2 GetScreenPosition() const;
	glm::ivec2 GetScreenPositionPixels() const;
	glm::ivec2 GetLocalPosition() const;
	glm::ivec2 GetCenteredLocalPosition() const;

	void Move(glm::vec2 vector);
	void CreateSprite(glm::vec2 position = glm::vec2(0.0f, 0.0f), 
					  glm::ivec2 size = glm::ivec2(32,32), 
					  const std::string& fileName = ".\\Default.png");

	float GetReloadTime() const;
	void ChangeWepon(int idx);
	int GetWeaponRange() const;
	int GetWeaponDmg() const;
	void Draw();
	void Shoot();
	
};

