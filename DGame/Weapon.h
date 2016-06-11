#pragma once
#include"Common.h"

class Weapon {
protected:
	int range;
	int dmg;
	int ammoNum;
	bool hasAmmo;
	std::string name;
	float reloadTime;
public:

	virtual std::string GetName() const { return name; }
	virtual int GetAmmoNum() const { return ammoNum; }
	virtual float GetReloadTime() const { return reloadTime; }
	virtual int GetRange() const { return range; }
	virtual int GetDamage() const { return dmg; }
	void Shoot(const glm::vec2& direction) {}
	Weapon();
	virtual ~Weapon();
};

class SniperRifle : public Weapon {


public:
	SniperRifle()
	{
		range = 3000;
		dmg = 80;
		name = "sniper rifle";
		hasAmmo = true;
		ammoNum = 10;
		reloadTime = 1.0f;
	}
	~SniperRifle() {}
};

class Glock : public Weapon {


public:
	Glock()
	{
		range = 100;
		dmg = 10;
		name = "Glock";
		hasAmmo = true;
		ammoNum = 20;
		reloadTime = 0.3f;
	}
	~Glock() {}
};
