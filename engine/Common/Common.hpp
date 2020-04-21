#pragma once

#include <glm/glm.hpp>

using byte_vec4 = glm::tvec4< uint8_t >;

constexpr int32_t TARGET_FPS = 144;
constexpr float TARGET_TIME = 1.0f / TARGET_FPS; // seconds
constexpr int32_t WIDTH = 1920;
constexpr int32_t HEIGHT = 1080;
constexpr int32_t NUM_FRAMES_TO_SAVE = 500;