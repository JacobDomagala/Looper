#version 460

#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_gpu_shader_int64 : enable

layout(set = 0, binding = 0) uniform UniformBufferObject
{
   mat4 u_projectionMat;
   mat4 u_viewMat;
   vec4 u_cameraPos;
   mat4 u_lightViewProj;
   vec4 u_lightDirection;
   vec4 u_lightColor;
   vec4 u_shadowParams;
}
ubo;

struct BufferData
{
   mat4 modelMat;
   vec4 color;
   vec4 texSamples;
};

layout(std430, set = 0, binding = 1) readonly buffer Block
{
   BufferData Transforms[];
};

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_texCoordDrawID;

layout(location = 0) out VS_OUT
{
   vec2 fTexCoord;

   flat int fDiffSampl;
   flat int fExtraSampl;
}
vs_out;

void
main(void)
{
   float drawID = a_texCoordDrawID.z;
   BufferData curInstanceData = Transforms[int(drawID)];

   vs_out.fTexCoord = a_texCoordDrawID.xy;
   vs_out.fDiffSampl = int(curInstanceData.texSamples.x);
   vs_out.fExtraSampl = int(curInstanceData.texSamples.y);

   gl_Position = ubo.u_lightViewProj * curInstanceData.modelMat * vec4(a_position.xyz, 1.0f);
}
