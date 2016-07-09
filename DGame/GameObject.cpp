#include"GameObject.h"
#include"Win_Window.h"

extern Win_Window* window;

GameObject::GameObject(const glm::vec2& pos, glm::ivec2 size, const std::string& sprite)
{
	collision = this->sprite.SetSpriteTextured(pos, size, sprite);
	globalPosition = pos;
	localPosition = glm::ivec2(pos.x, -pos.y);
	centeredGlobalPosition = this->sprite.GetCenteredPosition();
	drawMe = true;
}

glm::vec2 GameObject::GetScreenPositionPixels()
{
	glm::vec4 screenPosition = window->GetProjection() * glm::vec4(centeredGlobalPosition, 0.0f, 1.0f);
	glm::vec2 tmpPos = (glm::vec2(screenPosition.x, screenPosition.y) + glm::vec2(1.0f, 1.0f)) / glm::vec2(2.0f, 2.0f);
	tmpPos.x *= WIDTH;
	tmpPos.y *= -HEIGHT;
	tmpPos.y += HEIGHT;
	return tmpPos;
}