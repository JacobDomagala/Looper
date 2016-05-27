#pragma once
#include<Windows.h>
#include<iostream>
#include<string>
#include<vector>
#include<map>
#include <iterator> 
#include<fstream>
#include<memory>
#include<glew.h>
#include<glm.hpp>
#include<gtc\matrix_transform.hpp>
#include<gtc\type_ptr.hpp>
#include"SOIL.h"
#include<ft2build.h>
#include FT_FREETYPE_H  

typedef uint8_t uint8;
typedef uint32_t uint32;

struct charFour {
	uint8 x;
	uint8 y;
	uint8 z;
	uint8 w;
};

const float TARGET_FPS = 60.0f;
const float TARGET_TIME = 1.0f / TARGET_FPS;
const int WIDTH = 1280;
const int HEIGHT = 768;




