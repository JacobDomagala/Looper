#pragma once

#include <glm/glm.hpp>
#include <memory>

using byte_vec4 = glm::tvec4< uint8_t >;

constexpr float TARGET_FPS = 144.0f;
constexpr float TARGET_TIME = 1.0f / TARGET_FPS;
constexpr int32_t WIDTH = 1280;
constexpr int32_t HEIGHT = 768;
