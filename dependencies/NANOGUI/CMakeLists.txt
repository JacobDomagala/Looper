cmake_minimum_required(VERSION 3.13)

FIND_AND_DOWNLOAD_GIT_RESOURCE(NANOGUI https://github.com/wjakob/nanogui.git)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/NANOGUI)

file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/NANOGUI/nanogui/include/*.h")

add_library(NANOGUI2 INTERFACE IMPORTED GLOBAL)
target_sources(NANOGUI2 INTERFACE ${HEADERS})
target_include_directories(NANOGUI2 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/NANOGUI/include/" "${CMAKE_CURRENT_SOURCE_DIR}/NANOGUI/ext/eigen"
"${CMAKE_CURRENT_SOURCE_DIR}/NANOGUI/ext/glfw/include" "${CMAKE_CURRENT_SOURCE_DIR}/NANOGUI/ext/nanovg/src")
target_link_libraries (NANOGUI2 INTERFACE nanogui nanogui-obj glfw opengl32)