#pragma once
#include"Common.h"
#include"GameObject.h"
class Shaders;
class Player;

class Level {
	//IN PROGRESS

	static Sprite background;
	std::unordered_map<std::string, Texture> textures;
	Shaders shaders;


	
	glm::vec2 cameraPosition;
	glm::ivec2 cameraTilePos;
	
	glm::ivec2 playerPos;
	bool locked;

	glm::ivec2 tileSize;
	glm::ivec2 numTuilesOnScreen;
	glm::ivec2 tilesToDraw;
	static glm::ivec2 levelSize;
	
public:
	Level();
	~Level();
	std::vector<GameObject*> objects;
	// Helper functions
	static glm::vec2 GetLocalVec(glm::vec2 local) ;
	static glm::vec2 GetGlobalVec(glm::vec2 local) ;
	void MoveObjs(glm::vec2 moveBy, bool isCameraMovement = true);
	void Load(const std::string& fileName);
	void LoadPremade(const std::string& fileName, glm::ivec2 size);
	void LoadShaders(const std::string& shaderName);
	void AddGameObject(const glm::vec2& pos, glm::ivec2 size, const std::string& sprite);
	void Move(const glm::vec2& moveBy);
	void Draw();

	bool CheckCollision(const glm::ivec2& localPos, Player& player);
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

