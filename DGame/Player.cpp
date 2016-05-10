#include "Player.h"
#include"Win_Window.h"

#define ARRAY_COUNT(X) sizeof(X)/sizeof(X[0])

extern Win_Window* window;

Player::Player(const glm::vec2& position, const std::string& name):
	position(position),
	name(name)
{
	angle = 2.0f;
	translateVal = position;
}

void Player::CreateSprite(const glm::vec2& position, glm::ivec2 size, const std::string& fileName)
{
	sprite.SetSpriteTextured(position, size, fileName);
	this->position = sprite.GetCenteredPosition();
	//translateVal = this->position;
	program.LoadDefault();
}
void Player::LoadShaders(const std::string& shaderFile)
{
	program.LoadShaders("Shaders//" + shaderFile + "_vs.glsl", "Shaders//" + shaderFile + "_fs.glsl");
}
void Player::LoadShaders(const Shaders& program)
{
	this->program = program;	
}

glm::vec2 Player::GetGlobalPosition() const
{
	return position;
}

glm::vec2 Player::GetScreenPosition() const 
{
	glm::vec4 screenPosition = window->GetProjection() * glm::vec4(position, 0.0f, 1.0f);
	return glm::vec2(screenPosition.x, screenPosition.y);
}
void Player::Move(const glm::vec2& moveBy)
{
	sprite.Translate(moveBy);
	position += moveBy;
}

void Player::Draw()
{
#pragma region CURSOR_MATH
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	ScreenToClient(window->GetWindowHandle(), &cursorPos);
	glm::vec2 center(WIDTH / 2.0f, HEIGHT / 2.0f);
	cursorPos.x -= static_cast<LONG>(center.x);
	cursorPos.y -= static_cast<LONG>(center.y);
	float cursorX = cursorPos.x / center.x;
	float cursorY = cursorPos.y / center.y;

	glm::vec4 tmpCursor = window->GetProjection() * glm::vec4(cursorPos.x, cursorPos.y, 0.0f, 1.0f);
	glm::vec4 tmpPos = window->GetProjection() * glm::vec4(position, 0.0f, 1.0f);
	angle = -glm::degrees(glm::atan(tmpPos.y - tmpCursor.y, tmpPos.x - tmpCursor.x));

#pragma endregion

	sprite.Rotate(angle+90.0f);
//	sprite.Translate(translateVal);
	sprite.Render(program);

#pragma region LINIA

	//glm:: mat4 modelMatrix = glm::scale(glm::mat4(), glm::vec3(1.0f, 1.0f, 1.0f));
	//GLuint lineVertexArray;
	//GLuint lineVertexBuffer;
	//glGenVertexArrays(1, &lineVertexArray);
	//glGenBuffers(1, &lineVertexBuffer);
	//glBindVertexArray(lineVertexArray);
	//glBindBuffer(GL_ARRAY_BUFFER, lineVertexBuffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 2, 0, GL_DYNAMIC_DRAW);
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	//
	//
	////OutputDebugStringA((std::to_string(cursorX)+" "+std::to_string(cursorY)+"\n").c_str());
	//
	//glm::vec2 lineVerteces[2] = {
	//	glm::vec2(position.x,position.y),
	//	glm::vec2(cursorPos.x, cursorPos.y)
	//};
	//glBindVertexArray(lineVertexArray);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec2) * 2, lineVerteces);
	//
	//program.UseProgram();
	//program.SetUniformFloatMat4(modelMatrix, "modelMatrix");
	//program.SetUniformFloatVec4(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), "color");
	//
	//glDrawArrays(GL_LINES, 0, 2);
	//glBindVertexArray(0);
	//glDeleteBuffers(1, &lineVertexBuffer);
	//glDeleteVertexArrays(1, &lineVertexArray);

#pragma endregion
}
Player::~Player()
{
}
