#pragma once
#include"Common.h"

class Weapon {
	int ammoNum;
	bool hasAmmo;
	std::string name;
public:

	std::string GetGame() const { return name; }
	void Shoot(const glm::vec2& direction) {}
	Weapon();
	~Weapon();
};

