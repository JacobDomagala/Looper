cmake_minimum_required(VERSION 3.13)

FIND_AND_DOWNLOAD_GIT_RESOURCE(IMGUI https://github.com/ocornut/imgui.git)

set(HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/IMGUI/imconfig.h" "${CMAKE_CURRENT_SOURCE_DIR}/IMGUI/imgui.h" "${CMAKE_CURRENT_SOURCE_DIR}/IMGUI/imgui_internal.h" 
            "${CMAKE_CURRENT_SOURCE_DIR}/IMGUI/imstb_rectpack.h" "${CMAKE_CURRENT_SOURCE_DIR}/IMGUI/imstb_textedit.h" "${CMAKE_CURRENT_SOURCE_DIR}/IMGUI/imstb_truetype.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/IMGUI/examples/imgui_impl_glfw.h" "${CMAKE_CURRENT_SOURCE_DIR}/IMGUI/examples/imgui_impl_opengl3.h")

set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/IMGUI/imgui.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/IMGUI/imgui_demo.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/IMGUI/imgui_draw.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/IMGUI/imgui_widgets.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/IMGUI/examples/imgui_impl_glfw.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/IMGUI/examples/imgui_impl_opengl3.cpp")

add_library(IMGUI INTERFACE IMPORTED GLOBAL)
target_sources(IMGUI INTERFACE ${HEADERS} ${SOURCES})
target_include_directories(IMGUI INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/IMGUI/")