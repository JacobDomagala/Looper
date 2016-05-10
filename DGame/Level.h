#pragma once
#include"Common.h"
#include"GameObject.h"

class Shaders;

class Level {
	//IN PROGRESS
	Sprite background;
	std::map<std::string, Texture> textures; 
	Shaders shaders;

	std::vector<char> collisionMap;
	std::vector<GameObject> objects;
	
	glm::vec2 cameraPosition;
	glm::ivec2 cameraTilePos;
	
	glm::ivec2 playerPos;
	bool locked;

	glm::ivec2 tileSize;
	glm::ivec2 numTuilesOnScreen;
	glm::ivec2 tilesToDraw;
	glm::ivec2 levelSize;
public:
	Level();
	~Level();

	void Load(const std::string& fileName);
	void LoadPremade(const std::string& fileName, glm::ivec2 size);
	void LoadShaders(const std::string& shaderName);
	void LoadCollisionMap(const std::string& fileName);
	bool CheckCollision(const glm::vec2& position) const;
	void Move(const glm::vec2& moveBy);
	void Draw();
	
	void LockCamera() { locked = true; }
	void UnlockCamera() { locked = false; }
	bool IsCameraLocked() const { return locked; }
	glm::vec2 GetCameraPosition() { return cameraPosition; }
	glm::ivec2 GetCameraTiledPosition() { return cameraTilePos; }
	glm::vec2 GetLevelPosition() const { return background.GetPosition(); }
	glm::ivec2 GetSize() const { return levelSize; }
	void SetPlayersPosition(const glm::vec2& position);
	void MoveCamera(const glm::vec2 moveBy);
	glm::ivec2 CheckMoveCamera(const glm::vec2& moveBy) const;
	glm::ivec2 GetTilePosition(const glm::vec2& position) const;
};

