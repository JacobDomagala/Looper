#pragma once
#include"Common.h"
#include"Weapon.h"
#include"GameObject.h"

class Enemy : public GameObject{

	int maxHP;
	int currentHP;
	glm::ivec2 playerPositions[50];
	int currentIndex;
	Weapon* weapon;
public:
	int GetDmg() const { return weapon->GetDamage(); }
	void Shoot();
	bool GetState();
	void Hit(int dmg);
	void SetPlayerPos(glm::vec2 pos);
	Enemy(const glm::vec2& pos, glm::ivec2 size, const std::string& sprite);
	~Enemy();
};

