#pragma once
#include"Common.h"
#include"Shaders.h"
#include"Texture.h"

struct Character {
	GLuint textureID;   // ID handle of the glyph texture
	glm::ivec2 size;    // Size of glyph
	glm::ivec2 bearing;  // Offset from baseline to left/top of glyph
	GLuint advance;    // Horizontal offset to advance to next glyph
};

class Font {
	GLuint VAO, VBO;
	std::map<GLchar, Character> Characters;
	Shaders program;
	Texture texture;
public:
	Font() { }
	~Font() { }
	void SetFont(const std::string& fileName = "C:\\Windows\\Fonts\\arial.ttf");
	void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
};

