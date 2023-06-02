#pragma once

#include "types.hpp"

#include <glm/glm.hpp>

namespace looper {

constexpr float TARGET_FPS = 144.0f;
constexpr float TARGET_TIME = (1.0f / TARGET_FPS) * 1000.0f; // milliseconds
constexpr int32_t WIDTH = 1920;
constexpr int32_t HEIGHT = 1080;
constexpr int32_t NUM_FRAMES_TO_SAVE = 500;

} // namespace looper