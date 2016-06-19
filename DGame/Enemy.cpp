#include "Enemy.h"
#include"Timer.h"
#include"Game.h"
#include"Level.h"
#include"Weapon.h"

extern Timer* globalTimer;

void Enemy::Hit(int dmg)
{
	//currentHP -= dmg;
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
	globalPosition = pos;
	centeredGlobalPosition = this->sprite.GetCenteredPosition();
	localPosition = glm::ivec2(pos.x, -pos.y);
	drawMe = true;
	currentIndex = 0;
	maxHP = 100;
	currentHP = 100;
	weapon = new Glock();
}

void Enemy::SetPlayerPos(glm::vec2 pos)
{
	if(currentIndex != 49)
		playerPositions[currentIndex++] = pos;
	else
	{
		currentIndex = 1;
		playerPositions[0] = pos;
	}
}

void Enemy::Shoot()
{
	glm::ivec2 collided = Game::CheckBulletCollision(this, weapon->GetRange());
	//float len = glm::length(Game::player.GetGlobalPosition() - glm::vec2(centeredGlobalPosition));
	//Game::RenderText(std::to_string(collided.x) + "  " + std::to_string(collided.y), glm::vec2(0, 0), 1.0f, glm::vec3(1.0f, 0.0f, 1.0f));
	//Game::RenderText(std::to_string(len), glm::vec2(0, 100), 1.0f, glm::vec3(1.0f, 0.0f, 1.0f));
	if (collided != glm::ivec2(0, 0))
	{
		collided = Level::GetGlobalVec(collided);
		Game::DrawLine(centeredGlobalPosition, collided);
	}
	//Game::DrawLine(centeredGlobalPosition, Game::player.GetGlobalPosition());
}

Enemy::~Enemy()
{
}
