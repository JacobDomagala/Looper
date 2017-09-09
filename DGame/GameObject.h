#pragma once

#include "Common.h"
#include "Sprite.h"
#include "Shaders.h"

class GameObject 
{
protected:
	glm::vec2 globalPosition;
	glm::vec2 centeredGlobalPosition;
	
	glm::ivec2 localPosition;
	glm::ivec2 centeredLocalPosition;

	Sprite sprite;
	Shaders program;

	bool drawMe;
	std::shared_ptr<byte_vec4> collision;

	glm::mat4 translateMatrix;
	glm::vec2 translateVal;
	glm::mat4 rotateMatrix;
	glm::mat4 scaleMatrix;

public:
	// Constructors and destructors
	GameObject() = default;
	GameObject(const glm::vec2& pos, glm::ivec2 size, const std::string& sprite);
	virtual ~GameObject() = default;
	
	virtual void Hit(int dmg) {}
	virtual bool GetState() { return drawMe; }
	
	////SETERS
	glm::vec2 GetSize();				    //Get size of object
	glm::ivec2 GetCenteredLocalPosition();  //Get cenetered position in local(level wise) coords
	glm::vec2 GetCenteredGlobalPosition();	//Get centered position in global(OpenGL) coords
	glm::vec2 GetGlobalPosition();			//Get position in global (OpenGL) coords
	glm::ivec2 GetLocalPosition();			//Get position in local (level wise) coords
	glm::vec2 GetScreenPositionPixels();	//Get position in (0,0) to (WIDTH, HEIGHT) screen coords (0,0 BEING TOP LEFT CORNER)

	////GETERS
	void SetColor(const glm::vec3& color);
	void SetCenteredLocalPosition(glm::ivec2 pos);
	void SetLocalPosition(const glm::ivec2& position);
	void SetGlobalPosition(const glm::vec2& position);
	void SetShaders(const Shaders& program);
	void SetTexture(Texture texture);

	//Create sprite with default texture
	virtual void CreateSprite(const glm::vec2& position = glm::vec2(0.0f, 0.0f), glm::ivec2 size = glm::ivec2(10, 10));
	
	//Create sprite with texture from 'fileName'
	virtual void CreateSpriteTextured(const glm::vec2& position = glm::vec2(0.0f, 0.0f), glm::ivec2 size = glm::ivec2(16, 16), const std::string& fileName = ".\\Default.png");
	
	//Move object by 'moveBy'
	virtual void Move(const glm::vec2& moveBy, bool isCameraMovement = true);

	//Render object
	virtual void Render(const Shaders& program);
};

