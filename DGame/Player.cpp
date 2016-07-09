#include "Player.h"
#include"Win_Window.h"
#include"Enemy.h"
#include"Game.h"

#define ARRAY_COUNT(X) sizeof(X)/sizeof(X[0])

extern Win_Window* window;


Player::Player(glm::vec2 position, const std::string& name):
	globalPosition(position),
	name(name)
{
	maxHP = currentHP = 100;
	angle = 2.0f;
	translateVal = position;

	// not freeing the memory!
	currentWeapon = new SniperRifle;
	Weapon* tmp = new Glock;
	weapons[0] = currentWeapon;
	weapons[1] = tmp;
}

void Player::CreateSprite(glm::vec2 position, glm::ivec2 size, const std::string& fileName)
{
	collision = sprite.SetSpriteTextured(position, size, fileName);
	centeredGlobalPosition = sprite.GetCenteredPosition();
	localPosition = glm::ivec2(position.x, -position.y);
	//translateVal = this->position;
	program.LoadDefault();
}
void Player::LoadShaders(const std::string& shaderFile)
{
	program.LoadShaders("Shaders//" + shaderFile + "_vs.glsl", "Shaders//" + shaderFile + "_fs.glsl");
}
void Player::LoadShaders(const Shaders& program)
{
	this->program = program;	
}

bool Player::CheckCollision(glm::ivec2 localPos, GameObject* obj)
{
	glm::ivec2 size = sprite.GetSize();
	float length = glm::length(glm::vec2(localPos - centeredLocalPosition));
	if(length < size.x/2.5f)
		{
			currentHP -= ((Enemy*)obj)->GetDmg();
			sprite.SetColor(glm::vec3(1.0f, 0.0f, 0.0f));
			return false;
		}
	return true;
}
 glm::vec2 Player::GetGlobalPosition() const
{
	return globalPosition;
	
}
 glm::vec2 Player::GetCenteredGlobalPosition() const
{
	return centeredGlobalPosition;
}
 glm::ivec2 Player::GetLocalPosition() const
{
	return localPosition;
}
 glm::ivec2 Player::GetCenteredLocalPosition() const
{
	return centeredLocalPosition;
}

glm::vec2 Player::GetScreenPosition() const 
{
	glm::vec4 screenPosition = window->GetProjection() * glm::vec4(centeredGlobalPosition, 0.0f, 1.0f);
	return glm::vec2(screenPosition.x, screenPosition.y);
}
glm::ivec2 Player::GetScreenPositionPixels() const
{
	glm::vec4 screenPosition = window->GetProjection() * glm::vec4(centeredGlobalPosition, 0.0f, 1.0f);
	glm::vec2 tmpPos = (glm::vec2(screenPosition.x, screenPosition.y) + glm::vec2(1.0f,1.0f)) / glm::vec2(2.0f, 2.0f);
	tmpPos.x *= WIDTH;
	tmpPos.y *= -HEIGHT;
	tmpPos.y += HEIGHT;

	return tmpPos;
}

void Player::Move(glm::vec2 moveBy)
{
	sprite.Translate(moveBy);
	centeredGlobalPosition += moveBy;
	//localPosition += glm::ivec2(moveBy.x, -moveBy.y);
}

void Player::Draw()
{
#pragma region CURSOR_MATH
	//POINT cursorPos;
	//GetCursorPos(&cursorPos);
	//ScreenToClient(window->GetWindowHandle(), &cursorPos);
	//glm::vec2 center(WIDTH / 2.0f, HEIGHT / 2.0f);
	//cursorPos.x -= static_cast<LONG>(center.x);
	//cursorPos.y -= static_cast<LONG>(center.y);
	//float cursorX = cursorPos.x / center.x;
	//float cursorY = cursorPos.y / center.y;

	glm::vec2 cursorPos = window->GetCursorNormalized();

	//glm::vec4 tmpCursor = window->GetProjection() * glm::vec4(cursorPos.x, cursorPos.y, 0.0f, 1.0f);
	glm::vec4 tmpPos = window->GetProjection() * glm::vec4(centeredGlobalPosition, 0.0f, 1.0f);
	angle = -glm::degrees(glm::atan(tmpPos.y - cursorPos.y, tmpPos.x - cursorPos.x));

#pragma endregion
	
	/*for(int j = 0;j<128;++j)
		for (int i = 0; i < 128; ++i)
		{
			collision[i + j * 128] = glm::rotate(glm::mat4(), angle, glm::vec4(collision[i + j * 128]));
		}*/
	sprite.Rotate(angle+90.0f);
//	sprite.Translate(translateVal);
	sprite.Render(program);
	sprite.SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
	
	//Game::RenderText(std::to_string(globalPosition.x) + "   " + std::to_string(globalPosition.y), glm::vec2(0, 0), 1.0f, glm::vec3(1.0f, 0.2f, 0.1f));
}

void Player::Shoot()
{
	glm::ivec2 cursorPos = window->GetCursor();
	glm::ivec2 direction = cursorPos - localPosition;
	currentWeapon->Shoot(direction);
}
Player::~Player()
{
}

void Player::SetLocalPosition(glm::ivec2 pos)
{
	localPosition = pos;
}

void Player::SetCenteredLocalPosition(glm::ivec2 pos)
{
	centeredLocalPosition = pos;
	localPosition = glm::ivec2(centeredLocalPosition.x - sprite.GetSize().x/2, centeredLocalPosition.y - sprite.GetSize().y/2);
	globalPosition = Level::GetGlobalVec(localPosition);
	centeredGlobalPosition = Level::GetGlobalVec(centeredLocalPosition);
}

void Player::SetGlobalPosition(glm::vec2 pos)
{
	localPosition = pos;
}

 float Player::GetReloadTime() const
{
	return currentWeapon->GetReloadTime();
}

void Player::ChangeWepon(int idx)
{
	currentWeapon = weapons[idx];
}

 int Player::GetWeaponRange() const
{
	return currentWeapon->GetRange();
}

 int Player::GetWeaponDmg() const
{
	return currentWeapon->GetDamage();
}
