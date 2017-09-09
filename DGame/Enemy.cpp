#include "Enemy.h"
#include "Timer.h"
#include "Game.h"
#include "Level.h"
#include "Weapon.h"

extern Timer* globalTimer;

Enemy::Enemy(const glm::vec2& pos, const glm::ivec2& size, const std::string& sprite):
	GameObject(pos, size, sprite),
	maxHP(100),
	currentHP(maxHP),
	weapon(std::make_shared<Glock>()),
	combatStarted(false)
{ 

}

void Enemy::DealWithPlayer()
{
	// calculate distance between enemy and player
	float length = glm::length(m_centeredGlobalPosition - Game::player.GetCenteredGlobalPosition());
	
	// TODO: change this check with enemy's 'vision range'
	// player is enemy's sight of vision
	if (length <= 500.0f)
	{
		Shoot(); 
		SetPlayerPos(Game::player.GetCenteredLocalPosition());
	}
	// player is out of range, clear enemy's 'memory' 
	else
	{
		ClearPositions();
	}

	SetCenteredLocalPosition(Level::GetLocalVec(m_centeredGlobalPosition));
	SetLocalPosition(Level::GetLocalVec(m_globalPosition));
}

void Enemy::Hit(int dmg)
{
	//currentHP -= dmg;
	SetColor({ 1.0f, 0.0f, 0.0f });
}

bool Enemy::Visible() const
{
	if (currentHP <= 0)
		return false;

	return true;
}

void Enemy::SetPlayerPos(const glm::vec2& playerPos)
{
	timer.ToggleAndAccumulate();
	
	// make an enemy more human-like, prevent him from being perfect shooter
	// every half a second his aim will move a bit
	if (timer.GetAccumulator() > 0.5f)
	{
		timer.ResetAccumulator();

		// compute small offset value which simulates the 'aim wiggle'
		int xOffset = rand() % Game::player.GetSize().x + (-Game::player.GetSize().x/2);
		int yOffset = rand() % Game::player.GetSize().y + (-Game::player.GetSize().y/2);

		playerPosition = (playerPos + glm::vec2(xOffset, yOffset));
		combatStarted = true;
	}
}

void Enemy::Shoot()
{
	// prevent enemy shooting to some random position
	if (combatStarted)
	{
		glm::ivec2 collided = Game::CheckBulletCollision(this, Level::GetGlobalVec(playerPosition), weapon->GetRange());

		// if we hit anything draw a line 
		if (collided != glm::ivec2(0, 0))
		{
			Game::DrawLine(m_centeredGlobalPosition, Level::GetGlobalVec(collided));
		}
	}
}

void Enemy::ClearPositions()
{
	playerPosition = glm::vec2(0.0f, 0.0f);
	timer.ResetAccumulator();
	combatStarted = false;
}


