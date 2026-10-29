# Radiance cascades

Open `assets/levels/RadianceDemo.dgl` in the editor for a scene with warm and blue
emitters, a blocking wall, and a moving player light. The level has lighting enabled.

For another level:

1. Expand **Level → Lighting** and enable **Radiance cascades**.
2. Select an object and expand **Lighting**. Set its emission color and increase
   **Emission strength** (try 8).
3. Use **Light blocking** to control occlusion and **Receives light** to control
   whether the sprite is shaded. Backgrounds receive light without blocking it.
4. Adjust ambient light and light intensity in the level panel, then save. The
   same settings and materials are used in game mode.

Lighting is disabled for old levels unless explicitly enabled. Game objects block
and receive light by default; gizmos, pathfinder markers, grid lines, and ImGui
remain unlit. An emitter is visible to light rays even with light blocking set to
zero. Emission is tinted by the sprite's texture, mask, and color; transparent
texels and hidden sprites do not emit or occlude. Collision and light blocking
are independent settings.

## C++ API

```cpp
level.SetLightingSettings({true, 0.15f, 1.0f});

auto material = object.GetSprite().GetLightingMaterial();
material.emissionColor = {1.0f, 0.35f, 0.1f};
material.emissionStrength = 8.0f;
material.opacity = 1.0f;
material.receivesLight = true;
object.GetSprite().SetLightingMaterial(material);
```

Settings are stored in the level's optional `LIGHTING` object (`enabled`,
`ambient`, `intensity`). Each player, enemy, or object has an optional `lighting`
object (`emission_color`, `emission_strength`, `opacity`, `receives_light`).
Missing fields preserve defaults. Values are bounded and non-finite values are
replaced before they reach shaders. Sprite lighting is included in rewind state.

## Rendering

The implementation follows the 2D interval hierarchy described in
[Alexander Sannikov's radiance cascades paper](https://github.com/Raikiri/RadianceCascadesPaper).

* A separate raster pass captures premultiplied emission and opacity from the
  current sprites, back to front. It excludes non-blocking, non-emissive sprites.
* A jump-flood nearest-seed field accelerates ray traversal through empty space.
  Near geometry, rays walk individual cells and attenuate partially transparent
  cells once per traversal.
* Cascade zero has four directions and probes two lighting pixels apart. Each
  successive cascade doubles probe spacing and quadruples directional resolution
  and interval length. Intervals are contiguous: `[0,4]`, `[4,20]`, `[20,84]`, etc.
  The cascade count covers the lighting viewport's diagonal.
* Cascades merge from far to near with `L = Lnear + Tnear * Lfar` and
  `T = Tnear * Tfar`. Spatial interpolation stays within direction tiles, and
  each coarse angular sample averages four child directions.
* Cascade zero resolves to an angular average. The regular sprite shader applies
  `albedo * (ambient + intensity * irradiance + emission)` to receiving sprites.
  UI rendering occurs afterward with its own shader.

Lighting runs at half resolution, with the longest axis capped at 1024 pixels.
All lighting images and descriptor sets belong to a frame slot and application;
they are recreated with the render pipeline and released through VMA. Explicit
barriers synchronize capture, compute passes, and fragment sampling. Disabling
lighting skips the capture and compute dispatches.

This is screen-space emissive light transport, with no off-screen lighting,
surface normals, temporal accumulation, or recursive diffuse bounces. Occluders
are 2D silhouettes, so their interiors are dark apart from ambient/emission.
Small geometry can disappear at the reduced lighting resolution, and approximate
distance fields and probe interpolation can leak light near thin geometry.
Partially transparent blocking is evaluated per crossed lighting cell, so thicker
transparent objects attenuate more light.

## Build and verification

The Vulkan SDK's `glslc` is required. CMake compiles shaders into the build tree,
tracks source/include changes, and stops the build on shader errors. It does not
overwrite the legacy SPIR-V files in `assets/shaders`.

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

With `BUILD_TESTING=ON` (the default), tests cover settings persistence, legacy
defaults, invalid values, and cascade coverage at small, odd, and large viewport
sizes. A headless Vulkan test checks colored emission, wall occlusion, hidden
emitters, empty scenes, frame reuse, enable/disable, and resource recreation at
different sizes. It uses synchronization validation when the Khronos layer is
installed, and skips if there is no Vulkan runtime/device. It can run on Mesa's
software Vulkan implementation without a window.
