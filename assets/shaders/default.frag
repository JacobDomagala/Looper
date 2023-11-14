#version 460

#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_image_load_store : require
#extension GL_ARB_gpu_shader_int64 : enable

layout(set = 0, binding = 2) uniform sampler samp;
layout(set = 0, binding = 3) uniform texture2D textures[256];

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
    outColor = fs_in.fColor * (texture(sampler2D(textures[fs_in.fDiffSampl], samp), fs_in.fTexCoord) * texture(sampler2D(textures[fs_in.fExtraSampl], samp), fs_in.fTexCoord));
}
