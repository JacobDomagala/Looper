cmake_minimum_required(VERSION 3.13)

set(MODULE_NAME Engine)
project(${MODULE_NAME})

add_subdirectory(Common)
add_subdirectory(Window)
add_subdirectory(FileManager)
add_subdirectory(Logger)
add_subdirectory(Render)
add_subdirectory(Time)
add_subdirectory(Game)
add_subdirectory(Input)

add_library(${MODULE_NAME} INTERFACE)

target_include_directories(${MODULE_NAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries (${MODULE_NAME} INTERFACE
    #EXTERNALS
    NANOGUI2 OpenGL::GL GLEW::GLEW glm STB_IMAGE freetype
    #INTERNALS
    Window FileManager Logger Render Time Common Game Input)

target_compile_features(${MODULE_NAME} INTERFACE cxx_std_17)