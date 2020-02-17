#version 430 core

in vec2 fTexCoords;
uniform sampler2D texture;

out vec4 fColor;

float luma(vec4 vector)
{
	float tmp = 0.2126f * vector.r + 0.7152f * vector.g + 0.0722f * vector.b;
	return tmp;
}
vec4 Sobel()
{
	float treshhold = 2.0f;

	float dx = 1.0f/1280.0f;
	float dy = 1.0f/768.0f;

	float s00 = luma(texture2D(texture, fTexCoords + vec2(-dx, dy)));
	float s10 = luma(texture2D(texture, fTexCoords + vec2(-dx, 0.0f)));
	float s20 = luma(texture2D(texture, fTexCoords + vec2(-dx, -dy)));
	float s01 = luma(texture2D(texture, fTexCoords + vec2(0.0f, dy)));
	float s21 = luma(texture2D(texture, fTexCoords + vec2(0.0f, -dy)));
	float s02 = luma(texture2D(texture, fTexCoords + vec2(dx, dy)));
	float s12 = luma(texture2D(texture, fTexCoords + vec2(dx, 0.0f)));
	float s22 = luma(texture2D(texture, fTexCoords + vec2(dx, -dy)));

	float sx = s00 + 2 * s10 + s20 - (s02 + 2 * s12 + s22);
	float sy = s00 + 2 * s01 + s02 - (s20 + 2 * s21 + s22);

	float dist = sx * sx + sy * sy;
	if(dist > treshhold)
		return  vec4(1.0f);
	else
		return vec4(0.0f, 0.0f, 0.0f, 1.0f);

}

void main()
{
	fColor = texture2D(texture, fTexCoords);

	//fColor = Sobel();
}