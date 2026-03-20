#version 460

#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_image_load_store : require
#extension GL_ARB_gpu_shader_int64 : enable

layout(set = 0, binding = 2) uniform sampler2D textures[256];
layout(set = 0, binding = 3) uniform sampler2D shadowMap;

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

layout(location = 0) in VS_OUT
{
   vec4 fColor;
   vec2 fTexCoord;
   vec4 fLightSpacePos;

   flat int fDiffSampl;
   flat int fExtraSampl;
}
fs_in;

layout(location = 0) out vec4 outColor;

float
ComputeShadow(vec4 lightSpacePos)
{
    vec3 projCoords = lightSpacePos.xyz / max(lightSpacePos.w, 0.0001);
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z <= 0.0 || projCoords.z >= 1.0)
    {
        return 0.0;
    }

    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
    {
        return 0.0;
    }

    float shadow = 0.0;
    float bias = ubo.u_shadowParams.z;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float closestDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (projCoords.z - bias) > closestDepth ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}

void
main(void)
{
    vec4 base = texture(textures[fs_in.fDiffSampl], fs_in.fTexCoord);
    vec4 mask = texture(textures[fs_in.fExtraSampl], fs_in.fTexCoord);
    vec4 surface = fs_in.fColor * base * mask;

    if (surface.a <= 0.01)
    {
        discard;
    }

    vec3 lightDir = normalize(ubo.u_lightDirection.xyz);
    float diffuse = max(dot(vec3(0.0, 0.0, 1.0), lightDir), 0.0);
    float shadow = ComputeShadow(fs_in.fLightSpacePos);
    float ambient = ubo.u_shadowParams.x;
    float shadowStrength = ubo.u_shadowParams.y;
    float visibility = 1.0 - shadow * shadowStrength;

    vec3 lighting = vec3(ambient) + visibility * diffuse * ubo.u_lightColor.rgb;
    outColor = vec4(surface.rgb * lighting, surface.a);
}
