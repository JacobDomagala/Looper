cmake_minimum_required(VERSION 3.13)

set(MODULE_NAME Common)
project(${MODULE_NAME})

file(GLOB HEADERS "*.hpp")
file(GLOB SOURCES "*.cpp")

add_library(${MODULE_NAME} INTERFACE)

target_sources(${MODULE_NAME} INTERFACE ${SOURCES} ${HEADERS})
target_include_directories(${MODULE_NAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries (${MODULE_NAME} INTERFACE glm)
target_compile_features(${MODULE_NAME} INTERFACE cxx_std_17)