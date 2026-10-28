#pragma once

#include "lighting.hpp"
#include <nlohmann/json.hpp>

namespace looper::renderer {

inline float
ReadLightingNumber(const nlohmann::json& json, const char* key, float fallback)
{
   const auto entry = json.find(key);
   return entry != json.end() && entry->is_number() ? entry->get< float >() : fallback;
}

inline bool
ReadLightingBool(const nlohmann::json& json, const char* key, bool fallback)
{
   const auto entry = json.find(key);
   return entry != json.end() && entry->is_boolean() ? entry->get< bool >() : fallback;
}

inline LightingSettings
ReadLightingSettings(const nlohmann::json& json)
{
   LightingSettings settings;
   settings.enabled = ReadLightingBool(json, "enabled", settings.enabled);
   settings.ambient = ReadLightingNumber(json, "ambient", settings.ambient);
   settings.intensity = ReadLightingNumber(json, "intensity", settings.intensity);
   return SanitizeLighting(settings);
}

inline nlohmann::json
WriteLightingSettings(const LightingSettings& settings)
{
   return {{"enabled", settings.enabled},
           {"ambient", settings.ambient},
           {"intensity", settings.intensity}};
}

inline LightingMaterial
ReadLightingMaterial(const nlohmann::json& json, LightingMaterial material = {})
{
   const auto color = json.find("emission_color");
   if (color != json.end() && color->is_array() && color->size() == 3)
   {
      for (int i = 0; i < 3; ++i)
      {
         const auto& component = color->at(static_cast< size_t >(i));
         if (component.is_number())
         {
            material.emissionColor[i] = component.get< float >();
         }
      }
   }
   material.emissionStrength =
      ReadLightingNumber(json, "emission_strength", material.emissionStrength);
   material.opacity = ReadLightingNumber(json, "opacity", material.opacity);
   material.receivesLight = ReadLightingBool(json, "receives_light", material.receivesLight);
   return SanitizeLighting(material);
}

inline nlohmann::json
WriteLightingMaterial(const LightingMaterial& material)
{
   return {{"emission_color",
            {material.emissionColor.x, material.emissionColor.y, material.emissionColor.z}},
           {"emission_strength", material.emissionStrength},
           {"opacity", material.opacity},
           {"receives_light", material.receivesLight}};
}

} // namespace looper::renderer
