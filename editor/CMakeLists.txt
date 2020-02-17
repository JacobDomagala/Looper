set(MODULE_NAME Editor)

project(${MODULE_NAME})

# add_executable(${MODULE_NAME} "Main.cpp")
# target_link_libraries (${MODULE_NAME} PRIVATE nanogui INTERFACE SDL2 Engine)
# target_compile_features(${MODULE_NAME} PRIVATE cxx_std_11 INTERFACE cxx_std_17)

file(GLOB HEADERS "*.hpp")
file(GLOB SOURCES "*.cpp")

add_executable(${MODULE_NAME} ${HEADERS} ${SOURCES})
target_include_directories(${MODULE_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries (${MODULE_NAME} Engine)
target_compile_features(${MODULE_NAME} PUBLIC cxx_std_17)