#version 460

#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_image_load_store : require
#extension GL_ARB_gpu_shader_int64 : enable
#extension GL_ARB_shader_draw_parameters : enable

layout(set = 0, binding = 2) uniform sampler samp;
layout(set = 0, binding = 3) uniform texture2D textures[256];

layout(location = 0) in VS_OUT
{
   vec3 fPosition;
   vec2 fTexCoord;
   vec3 fColor;
   vec3 fCameraPosition;

   flat int fDiffSampl;
}
fs_in;

layout(location = 0) out vec4 outColor;

void
main(void)
{
    outColor = texture(sampler2D(textures[fs_in.fDiffSampl], samp), fs_in.fTexCoord);
}