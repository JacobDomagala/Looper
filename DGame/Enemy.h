#pragma once
#include"Common.h"
#include"Weapon.h"
#include"GameObject.h"

class Enemy : public GameObject{

	int maxHP;
	int currentHP;

	Weapon weapon;
public:
	Enemy();
	~Enemy();
};

