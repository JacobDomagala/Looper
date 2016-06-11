#include "Enemy.h"
#include"Timer.h"

extern Timer* globalTimer;

void Enemy::Hit(int dmg)
{
	currentHP -= dmg;
}

bool Enemy::GetState()
{
	if (currentHP <= 0)
		return false;

	return true;
}
Enemy::Enemy(const glm::vec2& pos, glm::ivec2 size, const std::string& sprite)
{
	collision = this->sprite.SetSpriteTextured(pos, size, sprite);
	this->position = pos;
	centeredPosition = this->sprite.GetCenteredPosition();
	drawMe = true;

	maxHP = 100;
	currentHP = 100;
}



Enemy::~Enemy()
{
}
