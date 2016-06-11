#pragma once
#include"Common.h"
#include"Level.h"
#include"Player.h"
#include"Font.h"
#include"Framebuffer.h"
#include"Timer.h"
#include"Win_Window.h"
extern Win_Window* window;
enum class GameState: char {
	MENU = 0,
	GAME,
	EDITOR
};

class DebugObject {
public:
	virtual void Draw() = 0;
	virtual ~DebugObject() {}
};
class Line : public DebugObject {
	glm::vec2 beg;
	glm::vec2 end;
	glm::vec3 color;
public:
	Line(glm::vec2 from, glm::vec2 to, glm::vec3 color)
	{
		beg = from;
		end = to;
		this->color = color;
	}
	void Draw() override
	{
		Shaders lineShader;
		lineShader.LoadShaders("lineVertex.glsl", "lineFragment.glsl");

		glm::vec2 vertices[2] = {
			beg,
			end
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
	~Line()
	{

	}
}		;
class Game {
	Timer timer;
	// framebuffer for first pass
	Framebuffer frameBuffer;
	float deltaTime;
	// all maps
	std::vector<std::string> levels;
	Level currentLevel;
	glm::ivec2 levelSize;

	charFour* collision;
	float shotLasttime;
	// state of the game 
	GameState state;

	
	glm::ivec2 playerPos;
	
	Font font;
	bool primaryFire;
	bool alternativeFire;


	glm::ivec2 CheckBulletCollision(int range);
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
	static std::vector<std::shared_ptr<DebugObject>> debugObjs;
	void RenderLine(glm::ivec2 collided, glm::vec3 color);
	
public:
	Game();
	static Player player;
	static void DrawLine(glm::vec2 from, glm::vec2 to, glm::vec3 color);
	void ProcessInput(float deltaTime);
	void RenderText(const std::string& text, const glm::vec2& position, float scale, const glm::vec3& color);
	void Render();
};

