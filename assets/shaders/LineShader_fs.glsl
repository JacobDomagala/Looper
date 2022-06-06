#version 420 core

layout(location = 0) out vec4 fColor;

in vec4 v_Color;

void main()
{
	fColor = v_Color;
}
