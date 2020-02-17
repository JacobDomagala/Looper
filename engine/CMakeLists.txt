cmake_minimum_required(VERSION 3.13)

set(MODULE_NAME Engine)
project(${MODULE_NAME})

# set(OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH})
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH})
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_PATH})

file(GLOB HEADERS "*.hpp")
file(GLOB SOURCES "*.cpp")

# add_library(${MODULE_NAME} OBJECT)
# target_link_directories(${MODULE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
# target_sources(${MODULE_NAME} PRIVATE ${SOURCES} PUBLIC ${HEADERS})
# target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
# target_link_libraries (${MODULE_NAME} PUBLIC NANOGUI SDL2 OpenGL::GL GLEW::GLEW freetype glm STB_IMAGE FileManager)

# UNCOMMENT FOR WHEN WE HAVE SUBMODULES
# target_link_libraries (${MODULE_NAME} INTERFACE Window Input)
add_subdirectory(Window)
# add_subdirectory(Input)
add_subdirectory(FileManager)

add_library(${MODULE_NAME} STATIC)

target_sources(${MODULE_NAME} PUBLIC ${SOURCES} ${HEADERS})
target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries (${MODULE_NAME} PUBLIC NANOGUI2 OpenGL::GL GLEW::GLEW glm Window STB_IMAGE freetype FileManager)
target_compile_features(${MODULE_NAME} PUBLIC cxx_std_17)