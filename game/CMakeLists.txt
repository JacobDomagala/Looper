set(MODULE_NAME Game)

project(${MODULE_NAME})

file(GLOB HEADERS "*.hpp")
file(GLOB SOURCES "*.cpp")

add_executable(${MODULE_NAME} WIN32 ${HEADERS} ${SOURCES})
target_include_directories(${MODULE_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries (${MODULE_NAME} PRIVATE Engine)
target_compile_features(${MODULE_NAME} PUBLIC cxx_std_17)