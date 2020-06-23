#version 330 core

layout(location = 0) out vec4 color;

in vec4 v_Color;
in vec2 v_TexCoord;
in float v_TexIndex;
in float v_TilingFactor;

uniform sampler2D u_Textures[32];

void main()
{
	color = v_Color * texture(u_Textures[int(v_TexIndex)], v_TexCoord * v_TilingFactor);;
}