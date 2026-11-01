#version 460

#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_gpu_shader_int64 : enable

// Only the radiance capture pipeline enables this specialization constant.
layout(constant_id = 0) const bool captureLighting = false;

layout(set = 0, binding = 0) uniform UniformBufferObject
{
   mat4 u_projectionMat;
   mat4 u_viewMat;
   vec4 u_cameraPos;
   vec4 u_lighting; // enabled, ambient, intensity, coverage scale
}
ubo;

struct BufferData
{
   mat4 modelMat;
   vec4 color;
   vec4 texSamples;
   vec4 emissionOpacity;
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
   flat vec4 fEmissionOpacity;
   flat float fReceivesLight;
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
   vs_out.fColor = curInstanceData.color;
   vs_out.fEmissionOpacity = curInstanceData.emissionOpacity;
   // The sign encodes receiving light without growing the per-instance SSBO stride.
   vs_out.fReceivesLight = curInstanceData.emissionOpacity.a >= 0.0 ? 1.0 : 0.0;
   if (vs_out.fReceivesLight < 0.5)
       vs_out.fEmissionOpacity.a = -curInstanceData.emissionOpacity.a - 1.0;

   vs_out.fDiffSampl = int(curInstanceData.texSamples.x);
   vs_out.fExtraSampl = int(curInstanceData.texSamples.y);

   mat4 modelMat = curInstanceData.modelMat;
   vec3 position = a_position;

   gl_Position = ubo.u_projectionMat * ubo.u_viewMat * modelMat * vec4(position.xyz, 1.0f);
   // Expand capture around the same camera center, preserving its zoom and rotation.
   if (captureLighting)
       gl_Position.xy /= max(ubo.u_lighting.w, 1.0);
}
