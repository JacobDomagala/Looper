cmake_minimum_required(VERSION 3.13)

set(MODULE_NAME Logger)

file(GLOB HEADERS "*.hpp")
file(GLOB SOURCES "*.cpp")

add_library(${MODULE_NAME} STATIC)

target_sources(${MODULE_NAME} PUBLIC ${SOURCES} ${HEADERS})
target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
#target_link_libraries (${MODULE_NAME} PUBLIC NANOGUI2 OpenGL::GL GLEW::GLEW glm Window STB_IMAGE freetype FileManager)
target_compile_features(${MODULE_NAME} PUBLIC cxx_std_17)