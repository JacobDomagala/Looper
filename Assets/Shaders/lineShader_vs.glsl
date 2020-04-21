#version 420 core

layout (location = 0) in vec2 vPosition;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
	gl_Position = projectionMatrix * vec4(vPosition, 0.0f, 1.0f);
}