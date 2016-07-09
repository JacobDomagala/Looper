#pragma once
#include"Common.h"
#include"Weapon.h"
#include"GameObject.h"
#include"Timer.h"
class Enemy : public GameObject{
	Timer timer;
	int maxHP;
	int currentHP;
	//const int arrayLen = 1;
	glm::vec2 playerPositions;
	int positionIdx;
	int shootIdx;
	Weapon* weapon;
public:
	int GetDmg() const { return weapon->GetDamage(); }
	void Shoot();
	void ClearPositions();
	bool GetState();
	void Hit(int dmg);
	void SetPlayerPos(glm::vec2 pos);
	Enemy(const glm::vec2& pos, glm::ivec2 size, const std::string& sprite);
	~Enemy();
};

