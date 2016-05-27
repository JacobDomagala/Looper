#include "GameObject.h"

GameObject::GameObject(const glm::vec2& pos, glm::ivec2 size, const std::string& sprite)
{
	collision = this->sprite.SetSpriteTextured(pos, size, sprite);
	this->position = pos;
}

bool GameObject::CheckCollision(const glm::ivec2& position) const
{
	glm::ivec2 tmpPos = glm::ivec2(localPosition.x, localPosition.y - 128);
	glm::ivec2 tmp = position - tmpPos;
	int width = 128;
	if (collision[tmp.x + tmp.y*width].w != 0)
		return false;

	return true;
}