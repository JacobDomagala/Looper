#include "Game.h"
#include"Win_Window.h"

extern Win_Window* window;
glm::vec2 destination;
glm::vec2 cursor;
glm::vec2 debug1;
#define myCeil(x) x < 0 ? floor(x) : ceil(x)

void swap(float& first, float& second)
{
	float tmp = first;
	first = second;
	second = tmp;
}
glm::ivec2 Game::CorrectPosition()
{
	
	uint32 linearPosition = static_cast<uint32>(floor(playerPos.x + playerPos.y*levelSize.x));
	uint32 linearLevelSize = levelSize.x * levelSize.y;

	charFour* tmpCollision = collision;
	
	glm::ivec2 tmpPosition = playerPos;
	glm::ivec2 returnVal = glm::ivec2();
	
	if (playerPos.x < 0)
	{
		returnVal.x = -playerPos.x;
		return returnVal;
	}
	if (playerPos.x >= levelSize.x)
	{
		returnVal.x = (levelSize.x-1) - playerPos.x;
		return returnVal;
	}
	if (playerPos.y < 0)
	{
		returnVal.y = -playerPos.y;
		return returnVal;
	}
	if (playerPos.y >= levelSize.y)
	{
		returnVal.y = (levelSize.y-1) - playerPos.y;
		return returnVal;
	}
	//if you stand where you're not supposed to due to fucking float -> int conversion (FeelsBadMan)
		if (tmpCollision[linearPosition].w != 0)
		{
			// the value of getting you out of shit < -5 ; 5 >
			int tmpval = 6;
			for (int j = -tmpval; j <= tmpval; ++j)
					{
						for (int i = -tmpval; i <= tmpval; ++i)
						{
							tmpPosition = playerPos + glm::ivec2(i, j);
							linearPosition = static_cast<uint32>(floor(tmpPosition.x + tmpPosition.y*levelSize.x));
							if ((tmpPosition.x > 0) && (tmpPosition.x < levelSize.x) &&
								(tmpPosition.y > 0) && (tmpPosition.y < levelSize.y) && 
								(tmpCollision[linearPosition].w == 0 ))
							{
								int yComp = tmpPosition.y;
								int xComp = tmpPosition.x;
								glm::ivec2 returnVal = glm::ivec2(xComp, yComp) - playerPos;
								return returnVal;
							}
						}
					}
		}
		return glm::ivec2(0, 0);
}
glm::vec2 Game::GetLocalVec(glm::vec2 local)
{
	glm::vec2 returnVal;
	returnVal = currentLevel.GetLevelPosition() - local;
	returnVal.y -= levelSize.y;
	returnVal *= -1;

	return returnVal;
}

glm::vec2 Game::GetGlobalVec(glm::vec2 local)
{
	glm::vec2 returnVal = local;
	returnVal *= -1;
	returnVal.y += levelSize.y;
	returnVal = currentLevel.GetLevelPosition() - returnVal;

	return returnVal;
}

void Game::RenderLine(glm::ivec2 collided, glm::vec3 color)
{
	glm::vec2 lineCollided = GetGlobalVec(collided);
		
	Shaders lineShader;
	lineShader.LoadShaders("lineVertex.glsl", "lineFragment.glsl");

	glm::vec2 vertices[2] = {
		glm::vec2(player.GetGlobalPosition()),
		glm::vec2(lineCollided)
	};
	glm::mat4 modelMatrix = glm::scale(glm::mat4(), glm::vec3(1.0f, 1.0f, 1.0f));
	GLuint lineVertexArray;
	GLuint lineVertexBuffer;
	glGenVertexArrays(1, &lineVertexArray);
	glGenBuffers(1, &lineVertexBuffer);
	glBindVertexArray(lineVertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, lineVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 2, vertices, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

	lineShader.UseProgram();
	lineShader.SetUniformFloatMat4(modelMatrix, "modelMatrix");
	lineShader.SetUniformFloatMat4(window->GetProjection(), "projectionMatrix");
	lineShader.SetUniformFloatVec4(glm::vec4(color, 1.0f), "color");

	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);
	glDeleteBuffers(1, &lineVertexBuffer);
	glDeleteVertexArrays(1, &lineVertexArray);
}
glm::ivec2 Game::CheckBulletCollision()
{
	float x1, x2, y1, y2;
	x1 = player.GetScreenPositionPixels().x;
	y1 = player.GetScreenPositionPixels().y;
	x2 = window->GetCursor().x;
	y2 = window->GetCursor().y;

	bool wasGreater = false;
	const bool steep = (fabs(y2 - y1) > fabs(x2 - x1));
	if (steep)
	{
		swap(x1, y1);
		swap(x2, y2);
	}

	if (x1 > x2)
	{
		wasGreater = true;
		swap(x1, x2);
		swap(y1, y2);
	}

	const float dx = x2 - x1;
	const float dy = fabs(y2 - y1);

	float error = dx / 2.0f;
	const int ystep = (y1 < y2) ? 1 : -1;
	int y = static_cast<int>(y1);

	const int maxX = (int)x2;
	int range = levelSize.x;
	for (int x = static_cast<int>(x1); x < maxX + range; x++)
	{
		if (steep)
		{
			//y,x 
			glm::ivec2 tmpPos = glm::ivec2();
			if(!wasGreater)
				tmpPos = playerPos + glm::ivec2(y-y1, x-x1);
			else
				tmpPos = playerPos - glm::ivec2(y - y1, x - x1);
			
			if (tmpPos.x == 0 || tmpPos.x == levelSize.x ||
				tmpPos.y == 0 || tmpPos.y == levelSize.y ||
				collision[tmpPos.x + tmpPos.y*levelSize.x].w != 0)
					return tmpPos;
			
		}
		else
		{
			//x,y
			glm::ivec2 tmpPos = glm::ivec2();
			if (!wasGreater)
				tmpPos = playerPos + glm::ivec2(x - x1, y - y1);
			else
				tmpPos = playerPos - glm::ivec2(x - x1, y - y1);
			
			if (tmpPos.x == 0 || tmpPos.x == levelSize.x ||
				tmpPos.y == 0 || tmpPos.y == levelSize.y ||
				collision[tmpPos.x + tmpPos.y*levelSize.x].w != 0)
					return tmpPos;
			
		}

		error -= dy;
		if (error < 0)
		{
			y += ystep;
			error += dx;
		}
	}
	return glm::ivec2();
}
glm::ivec2 Game::CheckCollision(glm::ivec2& moveBy)
{
	glm::ivec2 destination = playerPos + moveBy;
	int distance = glm::length(static_cast<glm::vec2>(moveBy));


	glm::vec2 tmpDirection = glm::normalize(glm::vec2(moveBy));
	glm::ivec2 direction = glm::ivec2(ceil(tmpDirection.x), ceil(tmpDirection.y));

	uint32 linearDestination = static_cast<uint32>(floor(destination.x + destination.y*levelSize.x));
	uint32 linearPosition = static_cast<uint32>(floor(playerPos.x + playerPos.y*levelSize.x));
	uint32 linearLevelSize = levelSize.x * levelSize.y;

	glm::ivec2 returnVal = glm::ivec2();
	if (destination.x < 0 || destination.x > levelSize.x ||
		destination.y < 0 || destination.y > levelSize.y)
		return returnVal;

	uint32 tmpPosition = linearPosition;
	returnVal = moveBy;
	charFour* tmpCollision = collision;

	// if you can move to destination (FeelsGoodMan)
	if (tmpCollision[linearDestination].w == 0)
	{
		return returnVal;
	}

	// if you're standing in shit, this will take it into account
	glm::ivec2 positionBias = glm::ivec2();

	// if you stand where you're not supposed to due to fucking float -> int conversion (FeelsBadMan)
	if (tmpCollision[linearPosition].w != 0)
	{
		// the value of getting you out of shit < -5 ; 5 >
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

							positionBias = glm::ivec2(xComp, yComp) - playerPos;
							goto next;
						}
					}
				}
	}
next:

	// if you couldn't get to shit, find closest in line point where you can go
	for (int i = distance; i > 0; --i)
	{
		glm::ivec2 tmpDest = playerPos + direction*i;
		tmpPosition = static_cast<uint32>(floor(tmpDest.x + tmpDest.y*levelSize.x));
		if ((tmpPosition > 0) && (tmpPosition < linearLevelSize) && 
			(tmpDest.x < levelSize.x) && (tmpDest.y < levelSize.y) && (tmpCollision[tmpPosition].w == 0))
		{
			returnVal = tmpDest - playerPos;
			returnVal += positionBias;
			return returnVal;
		}
	}

	// if you couldn't find anything in straight line, check the closest boundaries
	for (int j = distance; j > 0; --j)
	{
		if (direction.x == 0)
		{
			for (int i = -2; i <= 2; ++i)
			{
				glm::ivec2 tmpDirection = glm::ivec2(direction.x + i, direction.y);
				glm::ivec2 tmpDest = playerPos + tmpDirection*j;

				tmpPosition = static_cast<uint32>(floor(tmpDest.x + tmpDest.y*levelSize.x));
				if ((tmpDest.x > 0) && (tmpDest.y > 0) && 
					(tmpDest.x < levelSize.x) && (tmpDest.y < levelSize.y) && 
					(tmpCollision[tmpPosition].w == 0))
				{
					returnVal = tmpDest - playerPos;
					returnVal += positionBias;
					return returnVal;
				}
			}
		}
		else if (direction.y == 0)
		{
			for (int i = -2; i <= 2; ++i)
			{
				glm::ivec2 tmpDirection = glm::ivec2(direction.x, direction.y+i);
				glm::ivec2 tmpDest = playerPos + tmpDirection*j;

				tmpPosition = static_cast<uint32>(floor(tmpDest.x + tmpDest.y*levelSize.x));
				if ((tmpDest.x > 0) && (tmpDest.y > 0) &&
					(tmpDest.x < levelSize.x) && (tmpDest.y < levelSize.y) &&
					(tmpCollision[tmpPosition].w == 0))
				{
					returnVal = tmpDest - playerPos;
					returnVal += positionBias;
					return returnVal;
				}
			}
		}
	}

	// worst case scenario you won't move
	return glm::ivec2() + positionBias;
}
bool Game::CheckMove(glm::ivec2& moveBy)
{
	// compute the player's position relative to level

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
	if (glm::length(glm::vec2(playerMoveBy)))
	{
		currentLevel.Move(cameraMoveBy);
		if (CheckMove(playerMoveBy) == false)
			currentLevel.Move(-cameraMoveBy);
	}
}

void Game::MouseEvents(float deltaTime)
{
	float cameraMovement = 600.0f*deltaTime;
	
	POINT tmpCursor;
	GetCursorPos(&tmpCursor);
	ScreenToClient(window->GetWindowHandle(), &tmpCursor);
	glm::vec2 center(WIDTH / 2.0f, HEIGHT / 2.0f);
	tmpCursor.x -= static_cast<LONG>(center.x);
	tmpCursor.y -= static_cast<LONG>(center.y);
	float cursorX = tmpCursor.x / center.x;
	float cursorY = tmpCursor.y / center.y;
	window->SetCursor(glm::vec2(tmpCursor.x, tmpCursor.y));
	//glm::ivec2 tmp = CheckBulletCollision(glm::vec2(cursorX, -cursorY));
	destination = player.GetScreenPosition() * glm::vec2(WIDTH / 2, HEIGHT / 2);
	cursor = window->GetCursor();
	//debug1 = cursor - destination;

	//PRIMARY FIRE
	if (GetAsyncKeyState(VK_LBUTTON))
	{
		primaryFire = true;
		player.Shoot();
	}
	//ALTERNATIVE FIRE
	if (GetAsyncKeyState(VK_RBUTTON))
	{
		alternativeFire = true;
		player.Shoot();
	}
	if (cursorX > 0.8f)
	{
		currentLevel.Move(glm::vec2(-cameraMovement, 0.0f));
		player.Move(glm::vec2(-cameraMovement, 0.0f));
	}
	else if (cursorX < -0.8f)
	{
		currentLevel.Move(glm::vec2(cameraMovement, 0.0f));
		player.Move(glm::vec2(cameraMovement, 0.0f));
	}
	if (cursorY > 0.8f)
	{
		currentLevel.Move(glm::vec2(0.0f, -cameraMovement));
		player.Move(glm::vec2(0.0f, -cameraMovement));
	}
	else if (cursorY < -0.8f)
	{
		currentLevel.Move(glm::vec2(0.0f, cameraMovement));
		player.Move(glm::vec2(0.0f, cameraMovement));
	}
}
void Game::RenderFirstPass()
{
	frameBuffer.BeginDrawingToTexture();
	currentLevel.Draw();
	
	playerPos = GetLocalVec(player.GetGlobalPosition());
	glm::ivec2 correction = CorrectPosition();
 	player.Move(correction);
	correction = glm::ivec2();
	
	player.Draw();
	frameBuffer.EndFrawingToTexture();
}
void Game::RenderSecondPass()
{
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	//glm::ivec2 debug2 = CheckBulletCollision();
	//RenderLine(debug2);
	if (alternativeFire || primaryFire)
	{
		glm::vec2 tmp = CheckBulletCollision();
		glm::vec3 color = alternativeFire ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
		RenderLine(tmp, color);
		alternativeFire = primaryFire = false;
	}
	RenderText(std::to_string(deltaTime * 1000) + " ms", glm::vec2(static_cast<float>(-WIDTH / 2), static_cast<float>(-HEIGHT / 2) + 20), 0.4f, glm::vec3(1.0f, 0.0f, 1.0f)); 
	font.RenderText(std::to_string(playerPos.x) + " " + std::to_string(playerPos.y), 20, 20, 1, glm::vec3(1, .4, .6));
	//font.RenderText(std::to_string(debug2.x) + " " + std::to_string(debug2.y), 20, 60, 1, glm::vec3(1, .4, .6));
	//font.RenderText(std::to_string(cursor.x) + " " + std::to_string(cursor.y), 20, 140, 1, glm::vec3(1, .4, .6));
	//font.RenderText(std::to_string(debug1.x) + " " + std::to_string(debug1.y), 20, 100, 1, glm::vec3(1, .4, .6));
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
			unsigned char* tmpCollision = SOIL_load_image((folderPath + collisionMap).c_str(), &width, &height, NULL, SOIL_LOAD_RGBA);
			collision = (charFour*)tmpCollision;
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
			playerPos = glm::ivec2(playerX, -playerY);
		}
	}

	currentLevel.LoadPremade(folderPath + background, glm::ivec2(levelWidth, levelHeight));
	//currentLevel.LoadCollisionMap(folderPath + collisionMap);
	levelSize = currentLevel.GetSize();

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