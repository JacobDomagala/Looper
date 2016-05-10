#include "Game.h"
#include"Win_Window.h"

extern Win_Window* window;
glm::vec2 playerPos;
glm::ivec2 pixelPos;
bool Game::CheckBlock(glm::ivec2& moveBy)
{
	glm::ivec2 levelSize = currentLevel.GetSize();
	pixelPos = playerPos;
	glm::ivec2 destination = pixelPos + moveBy;
	glm::ivec2 distance = destination - pixelPos;

	uint32 linearDestination = static_cast<uint32>(floor(destination.x + destination.y*levelSize.x));
	uint32 linearPosition = static_cast<uint32>(floor(pixelPos.x + pixelPos.y*levelSize.x));

	if (linearDestination > levelSize.x * levelSize.y)
		return false;

	charFour* tmp = (charFour*)collision;
	if (tmp[linearDestination].w != 0)
	{
		for (int j = 0; j < distance.length(); ++j)
		{
			glm::ivec2 tmpDest = pixelPos + (moveBy / distance.length());
			linearDestination = static_cast<uint32>(floor(tmpDest.x + tmpDest.y*levelSize.x));
			if (tmp[linearDestination].w == 0)
			{

			}
		}
	}

	
	if (tmp[linearDestination].w == 0)
		return true;

	else return false;
}
bool Game::CheckBounds(glm::vec2& moveBy)
{
	glm::ivec2 levelSize = currentLevel.GetSize();

	// compute the player position relative to level
	playerPos = currentLevel.GetLevelPosition() - player.GetGlobalPosition();
	playerPos.y -= levelSize.y;
	playerPos *= -1.0f;
	
	glm::vec2 destination = playerPos + moveBy;

	if (destination.x > levelSize.x)
	{
		while (destination.x >= levelSize.x)
		{
			moveBy.x -= 0.01f;
			destination = playerPos + moveBy;
		}
	}
	if (destination.x < 0.0f)
	{
		while (destination.x <= 0.0f)
		{
			moveBy.x += 0.01f;
			destination = playerPos + moveBy;
		}
	}
	if (destination.y > levelSize.y)
	{
		while (destination.y >= levelSize.y)
		{
			moveBy.y -= 0.01f;
			destination = playerPos + moveBy;
		}
	}
	if (destination.y < 0.0f)
	{
		while (destination.y <= 0.0f)
		{
			moveBy.y += 0.01f;
			destination = playerPos + moveBy;
		}
	}

	if (CheckBlock(glm::ivec2(moveBy)))
	{
		player.Move(moveBy);
		return true;
	}
	return false;
}
void Game::KeyEvents(float deltaTime)
{
	float cameraMovement = 300.0f*deltaTime;
	float playerMovement = 500.0f*deltaTime;
	
	if (GetAsyncKeyState(VK_ESCAPE))
	{
		window->ShutDown();
	}
	if (GetAsyncKeyState('O'))
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	if (GetAsyncKeyState('P'))
	{
		glDisable(GL_BLEND);
	}
	if (GetAsyncKeyState('W'))
	{
		if (CheckBounds(glm::vec2(0, -playerMovement)))
		{
			currentLevel.Move(glm::vec2(0, cameraMovement));	
		}
	}
	if (GetAsyncKeyState('S'))
	{
		if (CheckBounds(glm::vec2(0.0f, playerMovement)))
		{
			currentLevel.Move(glm::vec2(0.0f, -cameraMovement));
		}
	}
	if (GetAsyncKeyState('A'))
	{
		if (CheckBounds(glm::vec2(-playerMovement, 0.0f)))
		{
			currentLevel.Move(glm::vec2(cameraMovement, 0.0f));
		}
	}
	if (GetAsyncKeyState('D'))
	{
		if (CheckBounds(glm::vec2(playerMovement, 0.0f)))
		{
			currentLevel.Move(glm::vec2(-cameraMovement, 0.0f));
		}
	}
	if (GetAsyncKeyState(VK_SPACE))
	{
		currentLevel.Move(-player.GetGlobalPosition());
		player.Move(-player.GetGlobalPosition());
	}
}

void Game::MouseEvents(float deltaTime)
{
	float cameraMovement = 600.0f*deltaTime;

	POINT cursorPos;
	GetCursorPos(&cursorPos);
	ScreenToClient(window->GetWindowHandle(), &cursorPos);
	if (cursorPos.x > WIDTH - 250)
	{
		currentLevel.Move(glm::vec2(-cameraMovement, 0.0f));
		player.Move(glm::vec2(-cameraMovement, 0.0f));
	}
	else if (cursorPos.x < 250)
	{
		currentLevel.Move(glm::vec2(cameraMovement, 0.0f));
		player.Move(glm::vec2(cameraMovement, 0.0f));
	}
	if (cursorPos.y > HEIGHT - 150)
	{
		currentLevel.Move(glm::vec2(0.0f, -cameraMovement));
		player.Move(glm::vec2(0.0f, -cameraMovement));
	}
	else if (cursorPos.y < 150)
	{
		currentLevel.Move(glm::vec2(0.0f, cameraMovement));
		player.Move(glm::vec2(0.0f, cameraMovement));
	}
}
void Game::RenderFirstPass()
{
	frameBuffer.BeginDrawingToTexture();
	currentLevel.Draw();
	player.Draw();
	frameBuffer.EndFrawingToTexture();
}
void Game::RenderSecondPass()
{
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	font.RenderText(std::to_string(pixelPos.x) + " " + std::to_string(pixelPos.y), 20, 20, 1, glm::vec3(1, .4, .6));
	RenderText(std::to_string(deltaTime*1000)+" ms", glm::vec2(static_cast<float>(-WIDTH / 2), static_cast<float>(-HEIGHT / 2)+20), 0.4f, glm::vec3(1.0f, 0.0f, 1.0f));
}
void Game::LoadLevel(const std::string& levelName)
{ 
	std::string folderPath = "Assets\\" + levelName + "\\";
	std::ifstream levelFile(folderPath + levelName + ".txt");
	if (!levelFile)
		window->ShowError("Can't open " + folderPath + levelName + ".txt", "Level loading");
	
	int levelWidth, levelHeight;
	std::string background;
	std::string collisionMap;

	while (!levelFile.eof())
	{
		std::string tmp = "";
		levelFile >> tmp;
		if (tmp == "Size:")
		{
			levelFile >> levelWidth;
			levelFile >> levelHeight;
		}
		else if (tmp == "Background:")
		{
			levelFile >> background;
		}
		else if (tmp == "Objects:")
		{
			// TODO KAPPA
		}
		else if (tmp == "Particles:")
		{
			// TODO KAPPA
		}
		else if (tmp == "First_pass_shaders:")
		{
			levelFile >> tmp;
			currentLevel.LoadShaders(tmp);
		}
		else if (tmp == "Second_pass_shaders:")
		{
			levelFile >> tmp;
			frameBuffer.LoadShaders(tmp);
		}
		else if (tmp == "Collision:")
		{
			levelFile >> collisionMap;
			int width, height;
			collision = SOIL_load_image((folderPath + collisionMap).c_str(), &width, &height, NULL, SOIL_LOAD_RGBA);
		}
		else if (tmp == "Player:")
		{
			int playerX, playerY;
			levelFile >> playerX;
			levelFile >> playerY;

			int playerWidth, playerHeight;
			levelFile >> playerWidth;
			levelFile >> playerHeight; 
			
			levelFile >> tmp;
			player.CreateSprite(glm::vec2(playerX, playerY), glm::ivec2(playerWidth, playerHeight), folderPath+tmp);
			levelFile >> tmp;
			player.LoadShaders(tmp);

		}
	}

	currentLevel.LoadPremade(folderPath + background, glm::ivec2(levelWidth, levelHeight));
	//currentLevel.LoadCollisionMap(folderPath + collisionMap);

	currentLevel.Move(-player.GetGlobalPosition());
	player.Move(-player.GetGlobalPosition());
}

Game::Game()
{
	font.SetFont();

	std::ifstream initFile("Assets\\GameInit.txt");
	if (!initFile)
		window->ShowError("Can't open Assets/GameInit.txt", "Game Initializing");

	while (!initFile.eof())
	{
		std::string tmp = "";
		initFile >> tmp;
		if (tmp == "Levels:")
		{
			while (initFile.peek() != '\n' && initFile.peek() != EOF)
			{
				initFile >> tmp;
				levels.push_back(tmp);
			}
		}

	}

	initFile.close();

	LoadLevel("Level1");
	state = GameState::GAME;
}

void Game::ProcessInput(float deltaTime)
{
	this->deltaTime = deltaTime;
	MouseEvents(deltaTime);
	KeyEvents(deltaTime);
}

void Game::RenderText(const std::string& text, const glm::vec2& position, float scale, const glm::vec3& color)
{
	font.RenderText(text, position.x, position.y, scale, color);
}

void Game::Render()
{
	RenderFirstPass();
	RenderSecondPass();
}