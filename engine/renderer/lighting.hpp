#pragma once

#include <glm/glm.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace looper::renderer {

// Opt in per level so existing levels retain their original appearance.
struct LightingSettings
{
   bool enabled = false;
   float ambient = 0.15f;
   float intensity = 1.0f;
   // Extra camera width/height captured on each side, independent of zoom.
   float offscreenMargin = 0.5f;
};

// Plain sprites (gizmos, pathfinder nodes, etc.) are unlit by default.
struct LightingMaterial
{
   glm::vec3 emissionColor = glm::vec3(1.0f);
   float emissionStrength = 0.0f;
   float opacity = 0.0f;
   bool receivesLight = false;
};

inline float
FiniteClamp(float value, float minimum, float maximum, float fallback)
{
   return std::isfinite(value) ? std::clamp(value, minimum, maximum) : fallback;
}

inline LightingSettings
SanitizeLighting(LightingSettings settings)
{
   settings.ambient = FiniteClamp(settings.ambient, 0.0f, 1.0f, 0.15f);
   settings.intensity = FiniteClamp(settings.intensity, 0.0f, 10.0f, 1.0f);
   settings.offscreenMargin = FiniteClamp(settings.offscreenMargin, 0.0f, 2.0f, 0.5f);
   return settings;
}

inline float
GetLightingCoverageScale(const LightingSettings& settings)
{
   return 1.0f + 2.0f * SanitizeLighting(settings).offscreenMargin;
}

inline LightingMaterial
SanitizeLighting(LightingMaterial material)
{
   for (int component = 0; component < 3; ++component)
   {
      material.emissionColor[component] =
         FiniteClamp(material.emissionColor[component], 0.0f, 1.0f, 1.0f);
   }
   material.emissionStrength = FiniteClamp(material.emissionStrength, 0.0f, 100.0f, 0.0f);
   material.opacity = FiniteClamp(material.opacity, 0.0f, 1.0f, 0.0f);
   return material;
}

struct CascadeLayout
{
   uint32_t width = 1;
   uint32_t height = 1;
   uint32_t count = 1;
   uint32_t atlasWidth = 2;
   uint32_t atlasHeight = 2;
};

// Half-resolution lighting, capped at 1024 pixels on the longest axis. Every
// cascade doubles probe spacing and quadruples both ray count and interval length.
inline CascadeLayout
GetCascadeLayout(uint32_t viewportWidth, uint32_t viewportHeight)
{
   const auto longest = std::max({viewportWidth, viewportHeight, 1u});
   const auto divisor = std::max(2u, (longest + 1023u) / 1024u);
   CascadeLayout layout;
   layout.width = std::max(1u, (viewportWidth + divisor - 1u) / divisor);
   layout.height = std::max(1u, (viewportHeight + divisor - 1u) / divisor);
   const float diagonal =
      std::hypot(static_cast< float >(layout.width), static_cast< float >(layout.height));
   float range = 4.0f;
   while (range < diagonal && layout.count < 6u)
   {
      range = range * 4.0f + 4.0f;
      ++layout.count;
   }
   for (uint32_t cascade = 0; cascade < layout.count; ++cascade)
   {
      const uint32_t spacing = 2u << cascade;
      layout.atlasWidth =
         std::max(layout.atlasWidth, ((layout.width + spacing - 1u) / spacing) * spacing);
      layout.atlasHeight =
         std::max(layout.atlasHeight, ((layout.height + spacing - 1u) / spacing) * spacing);
   }
   return layout;
}

} // namespace looper::renderer
