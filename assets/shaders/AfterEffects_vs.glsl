#version 430 core

layout (location = 0) in vec4 vPos;

out vec2 fTexCoords;

void main()
{
	vec4 positions[4] = vec4[4]( vec4(1.0f, 1.0f, 0.0f, 1.0f),
	vec4(-1.0f,  1.0f, 0.0f, 1.0f),
	vec4(-1.0f, -1.0f, 0.0f, 1.0f),
	vec4( 1.0f, -1.0f, 0.0f, 1.0f)
	);
	//gl_Position =  vec4(vPos.xy, 0.0f, 1.0f);
	//positions[gl_VertexID];

	vec2 vTexCoords[4] = vec2[4](vec2(1.0f, 1.0f),
	vec2(0.0f, 1.0f),
	vec2(0.0f, 0.0f),
	vec2(1.0f, 0.0f)
	);

	//fTexCoord = vTexCoords[gl_VertexID];

	gl_Position = vec4(vPos.xy, 0.0f, 1.0f);
	fTexCoords = vPos.zw;
}