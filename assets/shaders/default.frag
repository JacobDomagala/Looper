#version 460

#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_image_load_store : require
#extension GL_ARB_gpu_shader_int64 : enable

layout(set = 0, binding = 2) uniform sampler2D textures[256];
layout(set = 0, binding = 3) uniform sampler2D irradiance;
layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 projection;
    mat4 view;
    vec4 cameraPosition;
    vec4 lighting; // enabled, ambient, intensity, coverage scale
    vec4 viewport; // width, height, lighting capture UV offset
} ubo;

layout(location = 0) in VS_OUT
{
   vec4 fColor;
   vec2 fTexCoord;

   flat int fDiffSampl;
   flat int fExtraSampl;
   flat vec4 fEmissionOpacity;
   flat float fReceivesLight;
}
fs_in;

layout(location = 0) out vec4 outColor;

void
main(void)
{
    vec4 base = texture(textures[fs_in.fDiffSampl], fs_in.fTexCoord);
    vec4 mask = texture(textures[fs_in.fExtraSampl], fs_in.fTexCoord);
    outColor = fs_in.fColor * base * mask;
    if (outColor.a <= 0.001)
        discard;
    if (ubo.lighting.x > 0.5)
    {
        vec3 illumination = vec3(1.0);
        if (fs_in.fReceivesLight > 0.5)
        {
            // The visible viewport occupies the center of the expanded lighting capture.
            vec2 lightUV = (gl_FragCoord.xy / ubo.viewport.xy - 0.5) /
                           max(ubo.lighting.w, 1.0) + 0.5 + ubo.viewport.zw;
            illumination = vec3(ubo.lighting.y) + ubo.lighting.z *
                           texture(irradiance, lightUV).rgb;
        }
        outColor.rgb *= illumination + fs_in.fEmissionOpacity.rgb;
    }
}
