#version 460

layout(set = 0, binding = 2) uniform sampler2D textures[256];
layout(location = 0) in VS_OUT
{
    vec4 fColor;
    vec2 fTexCoord;
    flat int fDiffSampl;
    flat int fExtraSampl;
    flat vec4 fEmissionOpacity;
    flat float fReceivesLight;
} fs_in;
layout(location = 0) out vec4 outScene;

void main()
{
    vec4 surface = fs_in.fColor * texture(textures[fs_in.fDiffSampl], fs_in.fTexCoord)
                               * texture(textures[fs_in.fExtraSampl], fs_in.fTexCoord);
    vec3 emission = max(fs_in.fEmissionOpacity.rgb, vec3(0.0));
    // Emitters are visible to rays even when their authored blocking opacity is zero.
    float opacity = max(fs_in.fEmissionOpacity.a,
                        any(greaterThan(emission, vec3(0.0))) ? 1.0 : 0.0);
    if (surface.a <= 0.001 || opacity <= 0.001)
        discard;
    outScene = vec4(surface.rgb * emission * surface.a, opacity * surface.a);
}
