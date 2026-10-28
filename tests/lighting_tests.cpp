#include "renderer/lighting_json.hpp"

#include <cstdlib>
#include <iostream>
#include <limits>

using namespace looper::renderer;

namespace {
void
Require(bool condition, const char* message)
{
   if (!condition)
   {
      std::cerr << message << '\n';
      std::exit(EXIT_FAILURE);
   }
}
} // namespace

int
main()
{
   Require(!ReadLightingSettings({}).enabled, "Legacy levels must retain unlit rendering");
   const LightingSettings settings = {true, 0.2f, 2.0f};
   const auto restored = ReadLightingSettings(WriteLightingSettings(settings));
   Require(restored.enabled && restored.ambient == settings.ambient
              && restored.intensity == settings.intensity,
           "Settings must round trip");
   const LightingMaterial material = {{0.2f, 0.5f, 0.9f}, 12.0f, 0.75f, true};
   const auto restoredMaterial = ReadLightingMaterial(WriteLightingMaterial(material));
   Require(restoredMaterial.emissionColor == material.emissionColor
              && restoredMaterial.emissionStrength == material.emissionStrength
              && restoredMaterial.opacity == material.opacity && restoredMaterial.receivesLight,
           "Materials must round trip");
   const auto legacy = ReadLightingMaterial({}, material);
   Require(legacy.opacity == material.opacity && legacy.receivesLight,
           "Missing material fields must preserve object defaults");
   const auto invalid =
      ReadLightingSettings({{"enabled", "bad"}, {"ambient", -100.0f}, {"intensity", 10000.0f}});
   Require(!invalid.enabled && invalid.ambient == 0.0f && invalid.intensity == 10.0f,
           "Invalid settings must be bounded");
   auto invalidMaterial = material;
   invalidMaterial.emissionStrength = std::numeric_limits< float >::infinity();
   invalidMaterial.opacity = std::numeric_limits< float >::quiet_NaN();
   invalidMaterial = SanitizeLighting(invalidMaterial);
   Require(invalidMaterial.emissionStrength == 0.0f && invalidMaterial.opacity == 0.0f,
           "Non-finite material values must not enter shaders");
   const auto malformed = ReadLightingMaterial(
      {{"emission_color", {1, "invalid", -3}}, {"opacity", "invalid"}}, material);
   Require(malformed.emissionColor.y == material.emissionColor.y
              && malformed.emissionColor.z == 0.0f && malformed.opacity == material.opacity,
           "Malformed optional fields must be handled safely");

   for (const auto extent :
        {glm::uvec2{0, 0}, glm::uvec2{1, 1}, glm::uvec2{3, 29}, glm::uvec2{1919, 1079},
         glm::uvec2{1920, 1080}, glm::uvec2{7680, 4320}, glm::uvec2{16384, 1}})
   {
      const auto layout = GetCascadeLayout(extent.x, extent.y);
      Require(layout.width > 0 && layout.height > 0 && layout.width <= 1024
                 && layout.height <= 1024,
              "Lighting extent must be nonzero and bounded");
      float end = 0.0f;
      float length = 4.0f;
      for (uint32_t cascade = 0; cascade < layout.count; ++cascade)
      {
         const auto spacing = 2u << cascade;
         const auto columns = (layout.width + spacing - 1u) / spacing;
         const auto rows = (layout.height + spacing - 1u) / spacing;
         Require(columns * spacing <= layout.atlasWidth && rows * spacing <= layout.atlasHeight,
                 "Every directional tile must fit its atlas, including odd extents");
         end += length;
         length *= 4.0f;
      }
      Require(
         end >= std::hypot(static_cast< float >(layout.width), static_cast< float >(layout.height)),
         "Contiguous cascade intervals must cover the viewport diagonal");
   }
   std::cout << "Lighting settings, persistence, and cascade layout passed\n";
}
