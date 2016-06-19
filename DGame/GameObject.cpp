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

bool GameObject::CheckCollision(const glm::ivec2& position) const
{
	glm::ivec2 tmpPos = glm::ivec2(localPosition.x, localPosition.y - sprite.GetSize().y); //128
	glm::ivec2 tmp = position - tmpPos;
	int width = sprite.GetSize().x; //128
	if (collision[tmp.x + tmp.y*width].w != 0)
		return false;

	return true;
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