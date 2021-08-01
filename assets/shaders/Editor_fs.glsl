#version 430 core

in vec2 fTexCoords;

uniform sampler2D texture;
uniform vec4 color;
uniform vec2 objectSize;
uniform bool objectSelected;

out vec4 fColor;

void main()
{
	const float outlineWidthLeft = max(0.01f, min(objectSize.x * 0.0001f, 0.015f));
	const float outlineWidthRight = 1.0f - outlineWidthLeft;

	if(objectSelected)
	{
		if(fTexCoords.x <= outlineWidthLeft && fTexCoords.x >= 0.0f
			|| fTexCoords.y <= outlineWidthLeft && fTexCoords.y >= 0.0f
			|| fTexCoords.x <= 1.0f && fTexCoords.x >= outlineWidthRight
			|| fTexCoords.y <= 1.0f && fTexCoords.y >= outlineWidthRight)
		{
			fColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
		}
		else
		{
			fColor = texture2D(texture, fTexCoords) + vec4(0.5f, 0.0f, 0.2f, 0.1f);
		}
	}
	else
	{
		fColor = texture2D(texture, fTexCoords);
	}
}