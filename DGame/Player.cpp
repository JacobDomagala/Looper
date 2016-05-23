#include "Player.h"
#include"Win_Window.h"

#define ARRAY_COUNT(X) sizeof(X)/sizeof(X[0])

extern Win_Window* window;


Player::Player(const glm::vec2& position, const std::string& name):
	globalPosition(position),
	name(name)
{
	angle = 2.0f;
	translateVal = position;
}

void Player::CreateSprite(const glm::vec2& position, glm::ivec2 size, const std::string& fileName)
{
	sprite.SetSpriteTextured(position, size, fileName);
	this->globalPosition = sprite.GetCenteredPosition();
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

glm::vec2 Player::GetGlobalPosition() const
{
	return globalPosition;
}

glm::vec2 Player::GetScreenPosition() const 
{
	glm::vec4 screenPosition = window->GetProjection() * glm::vec4(globalPosition, 0.0f, 1.0f);
	return glm::vec2(screenPosition.x, screenPosition.y);
}
glm::ivec2 Player::GetScreenPositionPixels() const
{
	glm::vec4 screenPosition = window->GetProjection() * glm::vec4(globalPosition, 0.0f, 1.0f);
	glm::vec2 tmpPos = (glm::vec2(screenPosition.x, screenPosition.y) + glm::vec2(1.0f,1.0f)) / glm::vec2(2.0f, 2.0f);
	tmpPos.x *= WIDTH;
	tmpPos.y *= -HEIGHT;
	tmpPos.y += HEIGHT;

	return tmpPos;
}
void Player::Move(const glm::vec2& moveBy)
{
	sprite.Translate(moveBy);
	globalPosition += moveBy;
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
	glm::vec4 tmpPos = window->GetProjection() * glm::vec4(globalPosition, 0.0f, 1.0f);
	angle = -glm::degrees(glm::atan(tmpPos.y - cursorPos.y, tmpPos.x - cursorPos.x));

#pragma endregion

	sprite.Rotate(angle+90.0f);
//	sprite.Translate(translateVal);
	sprite.Render(program);

	

}

void Player::Shoot()
{
	glm::vec2 cursorPos = window->GetCursor();
	glm::vec2 direction = cursorPos - localPosition;
	currentWeapon.Shoot(direction);
}
Player::~Player()
{
}
