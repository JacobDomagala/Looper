#version 430 core

layout (location = 0) in vec4 vPos;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out vec2 fTexCoords;

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix *  vec4(vPos.xy, 0.0f, 1.0f);
	fTexCoords = vPos.zw;
}