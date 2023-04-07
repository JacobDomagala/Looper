#version 460

layout (push_constant) uniform PushConstants {
    vec4 color;
} pushConstants;

layout(location = 0) out vec4 outColor;

void
main(void)
{
    outColor = pushConstants.color;
}
