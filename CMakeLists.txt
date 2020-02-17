cmake_minimum_required(VERSION 3.13)

project(DGame)

set(OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(DEPENDENCIES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/dependencies)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_PATH})

add_subdirectory(dependencies)
add_subdirectory(engine)
add_subdirectory(game)
add_subdirectory(editor)

