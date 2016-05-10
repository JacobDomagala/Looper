#version 430 core

uniform sampler2D texture;
uniform vec4 color;
in vec2 fTex;

out vec4 fOutColor;

void main()
{
	vec4 computeAlpha = vec4(1.0f, 1.0f, 1.0f, texture2D(texture, fTex).r);
	fOutColor =  color * computeAlpha;

}