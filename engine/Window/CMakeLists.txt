cmake_minimum_required(VERSION 3.13)

set(MODULE_NAME Window)

project(${MODULE_NAME})

file(GLOB HEADERS "*.hpp")
file(GLOB SOURCES "*.cpp")

add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})

target_link_directories(${MODULE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries (${MODULE_NAME} PUBLIC
glm glfw OpenGL::GL GLEW::GLEW
Common Game Logger)