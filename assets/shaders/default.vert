#version 460

#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_gpu_shader_int64 : enable

layout(set = 0, binding = 0) uniform UniformBufferObject
{
   mat4 u_projectionMat;
   mat4 u_viewMat;
   vec4 u_cameraPos;
}
ubo;

struct BufferData
{
   mat4 modelMat;
   vec4 color;
   int diff;
};

layout(std430, set = 0, binding = 1) readonly buffer Block
{
   BufferData Transforms[];
};

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texCoord;
layout(location = 2) in float a_drawID;

layout(location = 0) out VS_OUT
{
   vec4 fColor;
   vec2 fTexCoord;

   flat int fDiffSampl;
}
vs_out;

void
main(void)
{
   BufferData curInstanceData = Transforms[int(a_drawID)];

   vs_out.fTexCoord = a_texCoord;
   vs_out.fColor = curInstanceData.color;

   vs_out.fDiffSampl = curInstanceData.diff;

   mat4 modelMat = curInstanceData.modelMat;
   //gl_Position = ubo.u_projectionMat * ubo.u_viewMat * vec4(a_position, 1.0f);
   gl_Position = ubo.u_projectionMat * ubo.u_viewMat * modelMat * vec4(a_position, 1.0f);
   // gl_Position = ubo.u_viewMat * modelMat * vec4(a_position, 1.0f);
}
