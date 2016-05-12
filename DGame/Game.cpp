#include "Game.h"
#include"Win_Window.h"

extern Win_Window* window;
glm::ivec2 playerPos;

glm::ivec2 Game::CheckCollision(glm::ivec2& moveBy)
{
 	glm::ivec2 levelSize = currentLevel.GetSize();
	glm::ivec2 destination = playerPos + moveBy;
	int distance = glm::length(static_cast<glm::vec2>(moveBy));


	glm::vec2 tmpDirection = glm::normalize(glm::vec2(moveBy));
	glm::ivec2 direction = glm::ivec2(ceil(tmpDirection.x), ceil(tmpDirection.y));

	uint32 linearDestination = static_cast<uint32>(floor(destination.x + destination.y*levelSize.x));
	uint32 linearPosition = static_cast<uint32>(floor(playerPos.x + playerPos.y*levelSize.x));
	uint32 linearLevelSize = levelSize.x * levelSize.y;

	glm::ivec2 returnVal = glm::ivec2();
	if (linearDestination > linearLevelSize || linearDestination < 0)
		return returnVal;

	uint32 tmpPosition = linearPosition;
	returnVal = moveBy;
	charFour* tmpCollision = (charFour*)collision;

	// if you can move to destination (FeelsGoodMan)
	if (tmpCollision[linearDestination].w == 0)
	{
		return returnVal;
	}

	// if you stand where you're not supposed to due to fucking float -> int conversion (FeelsBadMan)
	if (tmpCollision[linearPosition].w != 0)
	{
		// the value of getting your out of shit < -5 ; 5 >
		int tmpval = 5;
		for (int j = -tmpval; j <= tmpval; ++j)
				{
					for (int i = -tmpval; i <= tmpval; ++i)
					{
						tmpPosition = linearPosition + (i + j*levelSize.x);
						if ((tmpPosition < linearLevelSize) && (tmpPosition >= 0) && (tmpCollision[tmpPosition].w == 0 ))
						{
							int yComp = tmpPosition / levelSize.x;
							int xComp = tmpPosition - (yComp * levelSize.x);

							returnVal = glm::ivec2(xComp, yComp) - playerPos;
							return returnVal;
						}
					}
				}
	}

	// if you couldn't get to shit, find closest in line point where you can go
	for (int i = distance; i > 0; --i)
	{
		glm::ivec2 tmpDest = playerPos + direction*i;
		tmpPosition = static_cast<uint32>(floor(tmpDest.x + tmpDest.y*levelSize.x));
		if (tmpPosition > 0 && tmpPosition < linearLevelSize && tmpCollision[tmpPosition].w == 0)
		{
			returnVal = tmpDest - playerPos;
			return returnVal;
		}
	}

	// if you couldn't find anything in straight line, check the closest boundaries
	//for (int j = distance; j > 0; --j)
	//{
	//	
	//	for (int i = -1; i < 2; ++i)
	//	{
	//		glm::ivec2 tmpDirection = glm::ivec2(direction.x + i, direction.y);
	//		glm::ivec2 tmpDest = playerPos + tmpDirection*j;
	//	}
	//	if (direction.x == 0)
	//	{

	//	}
	//}



	// worst case scenario you won't move
	return glm::ivec2();
}
bool Game::CheckMove(glm::ivec2& moveBy)
{
	glm::ivec2 levelSize = currentLevel.GetSize();

	// compute the player's position relative to level
	playerPos = currentLevel.GetLevelPosition() - player.GetGlobalPosition();
	playerPos.y -= levelSize.y;
	playerPos *= -1;


	glm::ivec2 destination = playerPos + moveBy;

	if (destination.x > levelSize.x)
	{
		while (destination.x >= levelSize.x)
		{
			moveBy.x -= 1;
			destination = playerPos + moveBy;
		}
	}
	if (destination.x < 0)
	{
		while (destination.x <= 0)
		{
			moveBy.x += 1;
			destination = playerPos + moveBy;
		}
	}
	if (destination.y > levelSize.y)
	{
		while (destination.y >= levelSize.y)
		{
			moveBy.y -= 1;
			destination = playerPos + moveBy;
		}
	}
	if (destination.y < 0)
	{
		while (destination.y <= 0)
		{
			moveBy.y += 1;
			destination = playerPos + moveBy;
		}
	}

	// set the position to in bounds of the map, so it won't fuck up the collision later
	playerPos.x = playerPos.x >= 0 ? playerPos.x : 0;
	playerPos.y = playerPos.y >= 0 ? playerPos.y : 0;
	playerPos.x = playerPos.x < levelSize.x ? playerPos.x : levelSize.x - 1;
	playerPos.y = playerPos.y < levelSize.y ? playerPos.y : levelSize.y - 1;

	moveBy = CheckCollision(moveBy);
	player.Move(moveBy);
	if (glm::length(glm::vec2(moveBy)))
		return true;
	else return false;
}
void Game::KeyEvents(float deltaTime)
{
	float cameraMovement = 300.0f*deltaTime;
	float playerMovement = 500.0f*deltaTime;
	glm::ivec2 playerMoveBy = glm::ivec2();
	glm::ivec2 cameraMoveBy = glm::ivec2();

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
		playerMoveBy += glm::ivec2(0, -playerMovement);
		cameraMoveBy += glm::ivec2(0, cameraMovement);
	}
	if (GetAsyncKeyState('S'))
	{
		playerMoveBy += glm::ivec2(0, playerMovement);
		cameraMoveBy += glm::ivec2(0, -cameraMovement);
	}
	if (GetAsyncKeyState('A'))
	{
		playerMoveBy += glm::ivec2(-playerMovement, 0);
		cameraMoveBy += glm::ivec2(cameraMovement, 0);
	}
	if (GetAsyncKeyState('D'))
	{
		playerMoveBy += glm::ivec2(playerMovement, 0);
		cameraMoveBy += glm::ivec2(-cameraMovement, 0);
	}
	if (GetAsyncKeyState('R'))
	{
		player.Move(glm::vec2(200, 400));
	}
	if (GetAsyncKeyState(VK_SPACE))
	{
		currentLevel.Move(-player.GetGlobalPosition());
		player.Move(-player.GetGlobalPosition());
	}
	if(glm::length(glm::vec2(playerMoveBy)))
 		if (CheckMove(playerMoveBy))
		{
			currentLevel.Move(cameraMoveBy);
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
	font.RenderText(std::to_string(playerPos.x) + " " + std::to_string(playerPos.y), 20, 20, 1, glm::vec3(1, .4, .6));
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