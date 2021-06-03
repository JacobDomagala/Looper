cmake_minimum_required(VERSION 3.18)

project(DGame)

set(OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(DEPENDENCIES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/dependencies)
set(CMAKE_DIR_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_PATH})

add_definitions( -DCMAKE_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}" )

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

include(cmake/compile_warnings.cmake)
set_target_warnings(project_warnings)

# allow for static analysis options
include(cmake/static_analysis.cmake)

# Link this 'library' to use the static analysis tools and build options
add_library(project_options INTERFACE)

# sanitizer options if supported by compiler
include(cmake/sanitizer.cmake)
enable_sanitizers(project_options)

# Generate compile_commands.json to make it easier to work with clang based tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/util_functions.cmake)

add_subdirectory(dependencies)
add_subdirectory(engine)
add_subdirectory(game)
add_subdirectory(editor)

