#pragma once
#include"Common.h"
#include"Weapon.h"
#include"GameObject.h"

class Enemy : public GameObject{

	int maxHP;
	int currentHP;
	glm::ivec2 playerPositions[10];
	Weapon weapon;
public:
	bool GetState();
	void Hit(int dmg);
	Enemy(const glm::vec2& pos, glm::ivec2 size, const std::string& sprite);
	~Enemy();
};

