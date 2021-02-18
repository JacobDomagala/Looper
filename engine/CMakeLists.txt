cmake_minimum_required(VERSION 3.13)

set(MODULE_NAME Engine)
project(${MODULE_NAME})

file(GLOB_RECURSE HEADERS "*.hpp")
file(GLOB_RECURSE SOURCES "*.cpp")

add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})
target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/Common"
                                          "${CMAKE_CURRENT_SOURCE_DIR}/Core" "${CMAKE_CURRENT_SOURCE_DIR}/FileManager"
                                          "${CMAKE_CURRENT_SOURCE_DIR}/Game" "${CMAKE_CURRENT_SOURCE_DIR}/Input"
                                          "${CMAKE_CURRENT_SOURCE_DIR}/Logger" "${CMAKE_CURRENT_SOURCE_DIR}/Render"
                                          "${CMAKE_CURRENT_SOURCE_DIR}/Time" "${CMAKE_CURRENT_SOURCE_DIR}/Window")

target_link_libraries_system (${MODULE_NAME} PUBLIC glm glfw GLAD STB_IMAGE freetype nlohmann_json::nlohmann_json fmt IMGUI)
target_link_libraries(${MODULE_NAME} PRIVATE project_warnings)
target_compile_features(${MODULE_NAME} PRIVATE cxx_std_17)
