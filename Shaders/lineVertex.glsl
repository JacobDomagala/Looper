#version 420 core

layout (location = 0) in vec2 vPosition;

uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;

void main()
{
	gl_Position = projectionMatrix * modelMatrix * vec4(vPosition, 0.0f, 1.0f);
}