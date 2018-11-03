#pragma once

#include <memory>

struct byte_vec4
{
    union {
        struct
        {
            uint8_t x;
            uint8_t y;
            uint8_t z;
            uint8_t w;
        };

        struct
        {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
        };
    };
};

constexpr float   TARGET_FPS  = 60.0f;
constexpr float   TARGET_TIME = 1.0f / TARGET_FPS;
constexpr int32_t WIDTH       = 1280;
constexpr int32_t HEIGHT      = 768;
