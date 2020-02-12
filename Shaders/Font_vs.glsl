#version 430 core

layout (location = 0) in vec4 vPos;

uniform mat4 projectionMatrix;


out vec2 fTex;

void main()
{
	gl_Position = projectionMatrix * vec4(vPos.xy, 0.0f, 1.0f);
	fTex = vPos.zw;
}