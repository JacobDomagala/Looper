#version 430 core

in vec2 fTexCoords;

uniform sampler2D texture;
uniform float time;
uniform vec4 color;

out vec4 fColor;

void main()
{
	fColor = texture2D(texture, fTexCoords) * color;
	//fColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}