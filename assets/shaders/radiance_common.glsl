// Direction-first atlas: each tile contains the entire probe grid for one ray.
// Spatial interpolation is explicit so filtering can never cross direction tiles.
layout(push_constant) uniform CascadeParameters
{
    ivec2 extent;
    int cascade;
    int cascadeCount;
    int jump;
} params;

ivec2 probeCount(int cascade)
{
    int spacing = 2 << cascade;
    return (params.extent + spacing - 1) / spacing;
}

vec4 sampleCascade(sampler2D atlas, vec2 position, int direction, int cascade)
{
    int tiles = 2 << cascade;
    ivec2 probes = probeCount(cascade);
    ivec2 tile = ivec2(direction % tiles, direction / tiles) * probes;
    vec2 grid = position / float(tiles) - 0.5;
    ivec2 lower = ivec2(floor(grid));
    vec2 weight = fract(grid);
    ivec2 p00 = clamp(lower, ivec2(0), probes - 1);
    ivec2 p11 = clamp(lower + 1, ivec2(0), probes - 1);
    vec4 a = texelFetch(atlas, tile + p00, 0);
    vec4 b = texelFetch(atlas, tile + ivec2(p11.x, p00.y), 0);
    vec4 c = texelFetch(atlas, tile + ivec2(p00.x, p11.y), 0);
    vec4 d = texelFetch(atlas, tile + p11, 0);
    return mix(mix(a, b, weight.x), mix(c, d, weight.x), weight.y);
}
