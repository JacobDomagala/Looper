cmake_minimum_required(VERSION 3.13)

project(DGame)

set(OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(DEPENDENCIES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/dependencies)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_PATH})

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

include(cmake/compile_warnings.cmake)
set_target_warnings(project_warnings)

# allow for static analysis options
include(cmake/static_analysis.cmake)

# Generate compile_commands.json to make it easier to work with clang based tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(dependencies)
add_subdirectory(engine)
add_subdirectory(game)
add_subdirectory(editor)

