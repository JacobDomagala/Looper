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
   vec4 fColor;
   vec2 fTexCoord;

   flat int fDiffSampl;
   flat int fExtraSampl;
}
vs_out;

layout (push_constant) uniform PushConstants {
    float selectedIdx;
} pushConstants;

void
main(void)
{
   float drawID = a_texCoordDrawID.z;
   BufferData curInstanceData = Transforms[int(drawID)];

   vs_out.fTexCoord = a_texCoordDrawID.xy;
   vs_out.fColor = pushConstants.selectedIdx != drawID ? curInstanceData.color : vec4(0.4f, 0.1f, 0.2f, 1.0f);

   vs_out.fDiffSampl = int(curInstanceData.texSamples.x);
   vs_out.fExtraSampl = int(curInstanceData.texSamples.y);

   mat4 modelMat = curInstanceData.modelMat;
   vec3 position = a_position;

   gl_Position = ubo.u_projectionMat * ubo.u_viewMat * modelMat * vec4(position.xyz, 1.0f);
}
