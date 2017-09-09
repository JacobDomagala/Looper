#include "Sprite.h"
#include "Win_Window.h"

void Sprite::SetSprite(const glm::vec2& position , glm::ivec2 size)
{
	glGenVertexArrays(1, &vertexArrayBuffer);
	glGenBuffers(1, &vertexBuffer);
	glBindVertexArray(vertexArrayBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	glm::vec4 positions[6] = {
		glm::vec4(0.0f + size.x, 0.0f,          1.0f, 1.0f),
		glm::vec4(0.0f,          0.0f,          0.0f, 1.0f),
		glm::vec4(0.0f,          0.0f - size.y, 0.0f, 0.0f),

		glm::vec4(0.0f + size.x, 0.0f,		   1.0f, 1.0f),
		glm::vec4(0.0f,          0.0f - size.y, 0.0f, 0.0f),
		glm::vec4(0.0f + size.x, 0.0f - size.y, 1.0f, 0.0f)
	};
	this->centeredPosition.x = position.x + (size.x / 2.0f);
	this->centeredPosition.y = position.y - (size.y / 2.0f);
	this->position = position;
	this->size = size;

	translateVal = glm::vec3(position, 0.0f);
	angle = 0.0f;
	scaleVal = glm::vec2(1.0f, 1.0f);
	color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindVertexArray(0);
}

std::shared_ptr<byte_vec4> Sprite::SetSpriteTextured(const glm::vec2& position, glm::ivec2 size, const std::string& fileName)
{
	std::shared_ptr<byte_vec4> returnPtr = texture.LoadTextureFromFile(fileName);

	glGenVertexArrays(1, &vertexArrayBuffer);
	glGenBuffers(1, &vertexBuffer);
	glBindVertexArray(vertexArrayBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	glm::vec4 positions[6] = {
		glm::vec4(0.0f + size.x, 0.0f,          1.0f, 1.0f),
		glm::vec4(0.0f,          0.0f,          0.0f, 1.0f),
		glm::vec4(0.0f,          0.0f - size.y, 0.0f, 0.0f),

		glm::vec4(0.0f + size.x, 0.0f,		    1.0f, 1.0f),
		glm::vec4(0.0f,          0.0f - size.y, 0.0f, 0.0f),
		glm::vec4(0.0f + size.x, 0.0f - size.y, 1.0f, 0.0f)
	};
	this->centeredPosition.x = position.x + (size.x / 2.0f);
	this->centeredPosition.y = position.y - (size.y / 2.0f);
	this->position = position;
	this->size = size;

	translateVal = glm::vec3(position, 0.0f);
	angle = 0.0f;
	scaleVal = glm::vec2(1.0f, 1.0f);
	color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindVertexArray(0);

	return returnPtr;
}

void Sprite::Render(const Shaders& program)
{
	program.UseProgram();
	glBindVertexArray(vertexArrayBuffer);

	glm::mat4 modelMatrix;

	modelMatrix = glm::translate(modelMatrix, translateVal);
	modelMatrix = glm::translate(modelMatrix, glm::vec3((size.x / 2.0f)* scaleVal.x, (size.y / -2.0f) * scaleVal.y, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
	modelMatrix = glm::translate(modelMatrix, glm::vec3((size.x / -2.0f)* scaleVal.x, (size.y / 2.0f) * scaleVal.y, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(scaleVal, 1.0f));

	texture.Use(program.GetProgram());
	program.SetUniformFloatVec4(color, "color");
	program.SetUniformFloatMat4(Win_Window::GetInstance()->GetProjection(), "projectionMatrix");
	program.SetUniformFloatMat4(modelMatrix, "modelMatrix");

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}