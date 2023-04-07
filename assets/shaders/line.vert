#version 460

#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_gpu_shader_int64 : enable

layout(location = 0) in vec3 a_position;
layout(set = 0, binding = 0) uniform UniformBufferObject
{
   mat4 u_projectionMat;
   mat4 u_viewMat;
}
ubo;

void
main(void)
{
   gl_Position = ubo.u_projectionMat * ubo.u_viewMat * vec4(a_position, 1.0f);
}
