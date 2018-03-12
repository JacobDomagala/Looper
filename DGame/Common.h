#pragma once

#include "SOIL.h"
#include <Windows.h>
#include <array>
#include <fstream>
#include <ft2build.h>
#include <glew.h>
#include <glm.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include FT_FREETYPE_H

using uint8  = uint8_t;
using uint32 = uint32_t;

struct byte_vec4
{
    union {
        struct
        {
            uint8 x;
            uint8 y;
            uint8 z;
            uint8 w;
        };

        struct
        {
            uint8 r;
            uint8 g;
            uint8 b;
            uint8 a;
        };
    };
};

const float   TARGET_FPS  = 60.0f;
const float   TARGET_TIME = 1.0f / TARGET_FPS;
const int32_t WIDTH       = 1280;
const int32_t HEIGHT      = 768;
