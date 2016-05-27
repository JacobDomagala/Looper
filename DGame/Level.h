#pragma once
#include"Common.h"
#include"GameObject.h"

class Shaders;

class Level {
	//IN PROGRESS
	Sprite background;
	std::map<std::string, Texture> textures; 
	Shaders shaders;

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

	// Helper functions
	glm::vec2 GetLocalVec(glm::vec2 local);
	glm::vec2 GetGlobalVec(glm::vec2 local);

	void Load(const std::string& fileName);
	void LoadPremade(const std::string& fileName, glm::ivec2 size);
	void LoadShaders(const std::string& shaderName);
	void AddGameObject(const glm::vec2& pos, glm::ivec2 size, const std::string& sprite);
	void Move(const glm::vec2& moveBy);
	void Draw();
	bool CheckPosition(const glm::vec2& pos);
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

