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
	positionIdx = 0;
	shootIdx = 0;
	maxHP = 100;
	currentHP = maxHP;
	weapon = new Glock();
}

void Enemy::SetPlayerPos(glm::vec2 pos)
{
//	playerPositions = pos;
	timer.ToggleAndAccumulate();
	//Game::RenderText(std::to_string(timer.GetAccumulator()), glm::vec2(69, 69), 1.0f);
	if (timer.GetAccumulator() > 0.5f)
	{
		timer.ResetAccumulator();
	/*	if (positionIdx < arrayLen)
			playerPositions[positionIdx++] = pos;
		else
		{
			positionIdx = 0;
			playerPositions[0] = pos;
		}*/
		playerPositions = pos;

		// MAKE IT LOOK LIKE HE'S HAVING TROUBLE HITTING YOU
		// player size should be in L1 cache after first call
		int xOffset = rand() % Game::player.GetSize().x + (-Game::player.GetSize().x/2);
		int yOffset = rand() % Game::player.GetSize().y + (-Game::player.GetSize().y/2);
		glm::ivec2 offsetVal(xOffset, yOffset);
		playerPositions += offsetVal;
	}
}

void Enemy::Shoot()
{
	if (playerPositions == glm::vec2(0,0))
		return;
	// to prevent blank shooting to 0,0
	//if (shootIdx == positionIdx)
	//	return;
	
	glm::vec2 globalTmp = Level::GetGlobalVec(playerPositions);
	glm::ivec2 collided = Game::CheckBulletCollision(this, globalTmp, weapon->GetRange());
	
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
void Enemy::ClearPositions()
{
	shootIdx = 0;
	positionIdx = 0;
	playerPositions = glm::vec2();
}
Enemy::~Enemy()
{
}
