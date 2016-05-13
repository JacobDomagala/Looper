#include "Level.h"
#include"Shaders.h"
#include"Win_Window.h"
#include"Timer.h"

extern Win_Window* window;
Level::Level()
{
}

Level::~Level()
{
}

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
//		window->ShowError("Can't open file " + fileName, "Level loading error");
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
//		window->ShowError("Can't open file " + fileName, "Level loading error");
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


void Level::Move(const glm::vec2& moveBy)
{
	for (int i = 0; i < objects.size(); ++i)
	{
		objects[i].Move(moveBy);
	}
	background.Translate(moveBy);
	MoveCamera(moveBy);
}
void Level::Draw()
{
	background.Render(shaders);
	/*if (!objects.empty())
	{
		int xBegin = cameraTilePos.x - ceil(tilesToDrawX / 2.0f);
		int yBegin = cameraTilePos.y - ceil(tilesToDrawY / 2.0f);

		int xEnd = xBegin + tilesToDrawX;
		int yEnd = yBegin + tilesToDrawY;

		xBegin = xBegin >= 0 ? xBegin : 0;
		yBegin = yBegin >= 0 ? yBegin : 0;
		xEnd = xEnd <= levelWidth ? xEnd : levelWidth;
		yEnd = yEnd <= levelHeight ? yEnd : levelHeight;

		for (int j = yBegin; j < yEnd; ++j)
		{
			for (int i = xBegin; i < xEnd; ++i)
			{
				int index = i + j*levelWidth;
				objects[index].Render(shaders["GeneralShader"]);
			}
		}
	}*/
}

void Level::SetPlayersPosition(const glm::vec2& position)
{
	playerPos = position;
	playerPos /= tileSize;
}

void Level::MoveCamera(const glm::vec2 moveBy)
{
	cameraPosition -= moveBy;
	cameraTilePos = GetTilePosition(cameraPosition);
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

	int tileX = ceil(tmpX / static_cast<float>(tileSize.x));
	int tileY = ceil(tmpY / static_cast<float>(tileSize.y));

	return glm::ivec2(tileX, tileY);
}