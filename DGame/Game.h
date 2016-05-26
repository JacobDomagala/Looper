#pragma once
#include"Common.h"
#include"Level.h"
#include"Player.h"
#include"Font.h"
#include"Framebuffer.h"

enum class GameState: char {
	MENU = 0,
	GAME,
	EDITOR
};

struct charFour {
	uint8 x;
	uint8 y;
	uint8 z;
	uint8 w;
};
class Game {
	// framebuffer for first pass
	Framebuffer frameBuffer;
	float deltaTime;
	// all maps
	std::vector<std::string> levels;
	Level currentLevel;
	glm::ivec2 levelSize;

	charFour* collision;

	// state of the game 
	GameState state;

	Player player;
	glm::ivec2 playerPos;
	
	Font font;
	bool primaryFire;
	bool alternativeFire;
	// Helper functions
	glm::vec2 GetLocalVec(glm::vec2 local);
	glm::vec2 GetGlobalVec(glm::vec2 local);

	glm::ivec2 CheckBulletCollision();
	glm::ivec2 CheckCollision(glm::ivec2& moveBy);
	glm::ivec2 CorrectPosition();
	bool CheckMove(glm::ivec2& moveBy);
	void KeyEvents(float deltaTime);
	void MouseEvents(float deltaTime);

	// draws to framebuffer (texture)
	void RenderFirstPass();
	
	// draws to screen 
	void RenderSecondPass();
	void LoadLevel(const std::string& levelName);

//DEBUG
	void RenderLine(glm::ivec2 collided, glm::vec3 color);
	
public:
	Game();

	void ProcessInput(float deltaTime);
	void RenderText(const std::string& text, const glm::vec2& position, float scale, const glm::vec3& color);
	void Render();
};

