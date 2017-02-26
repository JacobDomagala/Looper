#include "GameObject.h"
#include "Win_Window.h"

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
	//Get the world coords 
	glm::vec4 screenPosition = window->GetProjection() * glm::vec4(centeredGlobalPosition, 0.0f, 1.0f);
	
	//convert from <-1,1> to <0,1>
	glm::vec2 tmpPos = (glm::vec2(screenPosition.x, screenPosition.y) + glm::vec2(1.0f, 1.0f)) / glm::vec2(2.0f, 2.0f);
	
	//convert from (0,0)->(1,1) [BOTTOM LEFT CORNER] to (0,0)->(WIDTH,HEIGHT) [TOP LEFT CORNER]
	tmpPos.x *= WIDTH;
	tmpPos.y *= -HEIGHT;
	tmpPos.y += HEIGHT;
	return tmpPos;
}
void GameObject::SetColor(const glm::vec3& color)
{
	sprite.SetColor(color);
}
glm::vec2 GameObject::GetSize()
{
	return sprite.GetSize();
}
glm::ivec2 GameObject::GetCenteredLocalPosition()
{
	return centeredLocalPosition;
}
void GameObject::SetCenteredLocalPosition(glm::ivec2 pos)
{
	centeredLocalPosition = pos;
}
void GameObject::SetLocalPosition(const glm::ivec2& position)
{
	localPosition = position;
}
void GameObject::SetGlobalPosition(const glm::vec2& position)
{
	globalPosition = position;
}
glm::vec2 GameObject::GetGlobalPosition()
{
	return globalPosition;
}
glm::ivec2 GameObject::GetLocalPosition()
{
	return localPosition;
}
glm::vec2 GameObject::GetCenteredGlobalPosition()
{
	return centeredGlobalPosition;
}
void GameObject::SetShaders(const Shaders& program)
{
	this->program = program;
}
void GameObject::SetTexture(Texture texture)
{
	sprite.SetTexture(texture);
}
void GameObject::CreateSprite(const glm::vec2& position, glm::ivec2 size)
{
	sprite.SetSprite(position, size);
	globalPosition = sprite.GetCenteredPosition();
}
void GameObject::CreateSpriteTextured(const glm::vec2& position, glm::ivec2 size, const std::string& fileName)
{
	collision = sprite.SetSpriteTextured(position, size, fileName);
	globalPosition = sprite.GetCenteredPosition();
}
void GameObject::Move(const glm::vec2& moveBy, bool isCameraMovement)
{
	sprite.Translate(moveBy);
	globalPosition += moveBy;
	centeredGlobalPosition += moveBy;
	//if (!isCameraMovement)
	//{
	//	localPosition += moveBy;
	//	centeredLocalPosition += moveBy;
	//}

}
void GameObject::Render(const Shaders& program)
{
	sprite.Render(program);
}