cmake_minimum_required(VERSION 3.13)

FIND_AND_DOWNLOAD_GIT_RESOURCE(STB_IMAGE https://github.com/nothings/stb.git)
add_library(STB_IMAGE INTERFACE IMPORTED GLOBAL)
target_include_directories(STB_IMAGE INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/STB_IMAGE")
