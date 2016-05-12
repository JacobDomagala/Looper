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
	unsigned char* collision;

	// state of the game 
	GameState state;

	Player player;
	
	Font font;
	glm::ivec2 CheckCollision(glm::ivec2& moveBy);
	bool CheckMove(glm::ivec2& moveBy);
	void KeyEvents(float deltaTime);
	void MouseEvents(float deltaTime);

	// draws to framebuffer (texture)
	void RenderFirstPass();

	// draws to screen 
	void RenderSecondPass();
	void LoadLevel(const std::string& levelName);
	
public:
	Game();
	void ProcessInput(float deltaTime);
	void RenderText(const std::string& text, const glm::vec2& position, float scale, const glm::vec3& color);
	void Render();
};

