#pragma once

#include "Common.h"
#include "Weapon.h"
#include "GameObject.h"
#include "Timer.h"

class Enemy : public GameObject
{
	// helper timer 
	Timer timer;

	// total HP
	int maxHP;

	// current HP
	int currentHP;

	// position where enemy will shoot
	glm::vec2 playerPosition;

	// current weapon
	std::shared_ptr<Weapon> weapon;

	// fight between enemy and player started
	bool combatStarted;

public:
	Enemy(const glm::vec2& pos, const glm::ivec2& size, const std::string& sprite);
	virtual ~Enemy() = default;
	
	virtual bool Visible() const override;
	virtual void Hit(int dmg) override;
	virtual void DealWithPlayer() override;

	int GetDmg() const 
	{ 
		return weapon->GetDamage(); 
	}
	
	void Shoot();
	void ClearPositions();
	void SetPlayerPos(const glm::vec2& pos);
};

