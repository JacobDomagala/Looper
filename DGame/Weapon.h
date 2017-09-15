#pragma once

#include "Common.h"

class Weapon 
{
protected:
	int m_range;
	int m_dmg;
	int m_ammoNum;
	bool m_hasAmmo;
	std::string m_name;
	float m_reloadTime;

	Weapon(int range, int dmg, int ammoNum, bool hasAmmo, std::string name, float reloadTime):
		m_range(range),
		m_dmg(dmg),
		m_ammoNum(ammoNum),
		m_hasAmmo(hasAmmo),
		m_name(name),
		m_reloadTime(reloadTime)
	{}

public:
	virtual ~Weapon() = default;
	
	virtual std::string GetName() const { return m_name; }
	virtual int GetAmmoNum() const { return m_ammoNum; }
	virtual float GetReloadTime() const { return m_reloadTime; }
	virtual int GetRange() const { return m_range; }
	virtual int GetDamage() const { return m_dmg; }
	virtual void Shoot(const glm::vec2& direction) = 0;
};

class SniperRifle : public Weapon 
{

public:
	SniperRifle():
		Weapon(3000, 80, 10, true, { "sniper rifle" }, 1.0f)
	{}
	virtual ~SniperRifle() = default;

	virtual void Shoot(const glm::vec2& direction) override
	{

	}
};

class Glock : public Weapon 
{
public:
	Glock():
		Weapon(3000, 10, 20, true, { "glock" }, 0.3f)
	{}
	virtual ~Glock() = default;

	virtual void Shoot(const glm::vec2& direction) override
	{

	}
};
