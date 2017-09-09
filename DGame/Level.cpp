#include "Level.h"
#include "Shaders.h"
#include "Player.h"
#include "Win_Window.h"
#include "Timer.h"
#include "Enemy.h"
#include "Game.h"

Sprite Level::background;
glm::ivec2 Level::levelSize;

//void Level::Load(const std::string& fileName)
//{
//	locked = false;
//	cameraPosition = glm::vec2(0.0f, 0.0f);
//	cameraTilePos = glm::ivec2(0,0);
//	this->tileWidth = tileWidth;
//	this->tileHeight = tileHeight;
//
//	std::ifstream initFile("Assets/LevelInit.txt");
//	if (!initFile.good())
//		Win_Window::GetInstance()->ShowError("Can't open file " + fileName, "Level loading error");
//
//	while (!initFile.eof())
//	{
//		std::string filePath = "";
//		while (initFile.peek() != ' ' && !initFile.eof())
//		{
//			char tmp;
//			initFile >> tmp;
//			filePath += tmp;
//		}
//		Texture tmp;
//		tmp.LoadTextureFromFile(filePath);
//		textures.insert(std::pair<std::string, Texture>(filePath, tmp));
//
//		// move 1 character since we used peek() to check for space
//		initFile.get();
//	}
//	initFile.close();
//
//
//	std::ifstream file(fileName);
//	if (!file.good())
//		Win_Window::GetInstance()->ShowError("Can't open file " + fileName, "Level loading error");
//	
//
//	// find end of line 
//	file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
//
//	// number of character till the end of line divided by two ( since we have spaces between objects )
//	levelWidth = file.tellg()/2 ;
//	file.seekg(0, file.beg);
//
//	int textureFileName;
//	int tilePosX = 0;
//	int tilePosY = 0;
//	while (!file.eof())
//	{
//		file >> textureFileName;
//
//		GameObject tmpObject;
//		if (textureFileName >= 0)
//		{
//			float tmpPosX =  tilePosX*tileWidth;
//			float tmpPosY =  tileHeight*tilePosY;
//			tmpObject.CreateSprite(glm::vec2(tmpPosX, tmpPosY), tileWidth, tileHeight);
//
//			tmpObject.SetTexture(textures[std::to_string(textureFileName)+".png"]);
//		//	tmpObject.Move(glm::vec2(0.0f, -2 * tileHeight));
//			objects.push_back(tmpObject);
//		}
//		
//		++tilePosX;
//		if (tilePosX == levelWidth)
//		{
//			tilePosX = 0;
//			++tilePosY;
//		}
//		
//	}
//	levelHeight = tilePosY;
//	numTilesOnScreenX = floor(WIDTH / static_cast<float>(tileWidth));
//	numTilesOnScreenY = floor(HEIGHT / static_cast<float>(tileHeight));
//
//	tilesToDrawX = numTilesOnScreenX + 2;
//	tilesToDrawY = numTilesOnScreenY + 2;
//
//	file.close();
//}

glm::vec2 Level::GetLocalVec(glm::vec2 global) 
{
	glm::vec2 returnVal;
	returnVal = background.GetPosition() - global;
	//returnVal.y = abs(returnVal.y);

	// change to 0.0 in top left 
	returnVal.y -= levelSize.y;
	returnVal *= -1;

	return returnVal;
}

glm::vec2 Level::GetGlobalVec(glm::vec2 local) 
{
	glm::vec2 returnVal = local;
	returnVal *= -1;
	returnVal.y += levelSize.y;
	returnVal = background.GetPosition() - returnVal;

	return returnVal;
}

bool Level::CheckCollision(const glm::ivec2& localPos, Player& player)
{
	if (objects.empty())
		return true;
	
	for (auto& obj : objects)
	{
		float length = glm::length(glm::vec2(localPos - obj->GetCenteredLocalPosition()));
		glm::vec2 objPos = obj->GetLocalPosition();
		glm::vec2 objSize = obj->GetSize();
		if(length < objSize.x/2.5f)
		{
			obj->Hit(player.GetWeaponDmg());
			obj->SetColor(glm::vec3(1.0f, 0.0f, 0.0f));
			
			return false;
		}
	}
	return true;
}

void Level::LoadPremade(const std::string& fileName, glm::ivec2 size)
{
	locked = false;
	cameraPosition = glm::vec2(0.0f, 0.0f);
	cameraTilePos = glm::ivec2(0, 0);
	this->levelSize = size;

	background.SetSpriteTextured(glm::vec2(0, 0), size, fileName);
	shaders.LoadDefault();
}

void Level::LoadShaders(const std::string& shaderName)
{
	shaders.LoadShaders("Shaders//" + shaderName + "_vs.glsl", "Shaders//" + shaderName + "_fs.glsl");
}

void Level::AddGameObject(const glm::vec2& pos, glm::ivec2 size, const std::string& sprite)
{
	std::unique_ptr<Enemy> tmpObj = std::make_unique<Enemy>(pos, size, sprite);
	glm::ivec2 tmpPos = tmpObj->GetCenteredGlobalPosition();
	tmpPos.y *= -1;
	tmpObj->SetCenteredLocalPosition(tmpPos);
	objects.push_back(std::move(tmpObj));
}

void Level::Move(const glm::vec2& moveBy)
{
	for (auto& obj : objects)
	{
		obj->Move(moveBy);
	}
	background.Translate(moveBy);
	MoveCamera(moveBy);
}

void Level::Draw()
{
	background.Render(shaders);
	if (!objects.empty())
	{
		/*	int xBegin = cameraTilePos.x - ceil(tilesToDrawX / 2.0f);
			int yBegin = cameraTilePos.y - ceil(tilesToDrawY / 2.0f);*/

			//int xEnd = xBegin + tilesToDrawX;
			//int yEnd = yBegin + tilesToDrawY;

			//xBegin = xBegin >= 0 ? xBegin : 0;
			//yBegin = yBegin >= 0 ? yBegin : 0;
			//xEnd = xEnd <= levelSize.x ? xEnd : levelSize.x;
			//yEnd = yEnd <= levelSize.y ? yEnd : levelSize.y;

			//for (int j = yBegin; j < yEnd; ++j)
			//{
			//	for (int i = xBegin; i < xEnd; ++i)
			//	{
			//		int index = i + j*levelSize.x;
			//		objects[index].Render(shaders["GeneralShader"]);
			//	}
			//}
		
		//for (auto& obj : objects)
		//{
		//	Enemy* objE = std::dynamic_pointer_cast<Enemy*>(obj.get());
		//	if (objE->GetState())
		//	{
		//		
		//		glm::vec2 tmpL = objE->GetCenteredGlobalPosition() - Game::player.GetCenteredGlobalPosition();
		//		float tmpLF = glm::length(tmpL);

		//		if (tmpLF <= 500.0f)
		//		{
		//			objE->Shoot();
		//		}
		//		else
		//			((Enemy*)objE)->ClearPositions();

		//		if (tmpLF <= 800.0f)
		//			objE->SetPlayerPos(Game::player.GetCenteredLocalPosition());
		//		
		//		
		//		obj->SetCenteredLocalPosition(GetLocalVec(objE->GetCenteredGlobalPosition()));
		//		obj->SetLocalPosition(GetLocalVec(objE->GetGlobalPosition()));
		//		obj->Render(shaders);
		//		obj->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
		//		
		//	}
		//}

	}
}

void Level::MoveObjs(glm::vec2 moveBy, bool isCameraMovement)
{
	for (auto& obj : objects)
	{
		obj->Move(moveBy, isCameraMovement);
	}
}

void Level::SetPlayersPosition(const glm::vec2& position)
{
	playerPos = position;
	playerPos /= tileSize;
}

void Level::MoveCamera(const glm::vec2 moveBy)
{
	cameraPosition -= moveBy;
	//cameraTilePos = GetTilePosition(cameraPosition);
}

glm::ivec2 Level::CheckMoveCamera(const glm::vec2& moveBy) const
{
	glm::vec2 tmp = cameraPosition;
	tmp -= moveBy;

	return GetTilePosition(tmp);
}

glm::ivec2 Level::GetTilePosition(const glm::vec2& position) const
{
	float tmpX = position.x;
	float tmpY = position.y;

	int tileX = static_cast<int>(ceilf(tmpX / static_cast<float>(tileSize.x)));
	int tileY = static_cast<int>(ceilf(tmpY / static_cast<float>(tileSize.y)));

	return glm::ivec2(tileX, tileY);
}