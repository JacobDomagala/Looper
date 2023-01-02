#version 460

#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_image_load_store : require
#extension GL_ARB_gpu_shader_int64 : enable

layout(set = 0, binding = 2) uniform sampler samp;
layout(set = 0, binding = 3) uniform texture2D textures[256];

layout(location = 0) in VS_OUT
{
   vec2 fTexCoord;
   vec3 fColor;

   flat int fDiffSampl;
}
fs_in;

layout(location = 0) out vec4 outColor;

void
main(void)
{
    outColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);//texture(sampler2D(textures[fs_in.fDiffSampl], samp), fs_in.fTexCoord);
}
