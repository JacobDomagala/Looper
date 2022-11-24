#version 460

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_uv;
layout (location = 2) in vec4 a_color;

layout (push_constant) uniform PushConstants {
	vec2 scale;
	vec2 translate;
} pushConstants;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outColor;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	outUV = a_uv;
	outColor = a_color;
	gl_Position = vec4(a_pos * pushConstants.scale + pushConstants.translate, 0.0, 1.0);
}
