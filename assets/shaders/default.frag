#version 460

#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_image_load_store : require
#extension GL_ARB_gpu_shader_int64 : enable

layout(set = 0, binding = 2) uniform sampler2D textures[256];

layout(location = 0) in VS_OUT
{
   vec4 fColor;
   vec2 fTexCoord;

   flat int fDiffSampl;
   flat int fExtraSampl;
}
fs_in;

layout(location = 0) out vec4 outColor;

void
main(void)
{
    vec4 base = texture(textures[fs_in.fDiffSampl], fs_in.fTexCoord);
    vec4 mask = texture(textures[fs_in.fExtraSampl], fs_in.fTexCoord);
    outColor = fs_in.fColor * base * mask;
}
