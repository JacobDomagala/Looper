set(MODULE_NAME Editor)

project(${MODULE_NAME})

file(GLOB_RECURSE HEADERS "*.hpp")
file(GLOB_RECURSE SOURCES "*.cpp")

add_executable(${MODULE_NAME} ${HEADERS} ${SOURCES})
target_include_directories(${MODULE_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/gui")

target_link_libraries (${MODULE_NAME} Engine)
target_compile_features(${MODULE_NAME} PRIVATE cxx_std_20)
