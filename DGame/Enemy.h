#pragma once
#include"Common.h"
#include"Weapon.h"
class Enemy {
	glm::vec2 position;
	Weapon weapon;
public:
	Enemy();
	~Enemy();
};

