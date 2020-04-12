#version 430 core

in vec2 fTexCoords;

uniform sampler2D texture;
uniform vec4 color;

uniform vec2 objectSize;

out vec4 fColor;

void main()
{

	const float outlineWidthLeft = max(0.008f, min(objectSize.x * 0.0001f, 0.008f));
	const float outlineWidthRight = 1.0f - outlineWidthLeft;

	if(fTexCoords.x <= outlineWidthLeft && fTexCoords.x >= 0.0f
		|| fTexCoords.y <= outlineWidthLeft && fTexCoords.y >= 0.0f
		|| fTexCoords.x <= 1.0f && fTexCoords.x >= outlineWidthRight
		|| fTexCoords.y <= 1.0f && fTexCoords.y >= outlineWidthRight)
	{
		fColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else
	{
		fColor = texture2D(texture, fTexCoords) * color + vec4(0.5f, 0.0f, 0.2f, 0.1f);
	}
}