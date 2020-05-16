set(MODULE_NAME Editor)

project(${MODULE_NAME})

file(GLOB_RECURSE HEADERS "*.hpp")
file(GLOB_RECURSE SOURCES "*.cpp")

add_executable(${MODULE_NAME} ${HEADERS} ${SOURCES})
target_include_directories(${MODULE_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/GUI"
                                                  "${CMAKE_CURRENT_SOURCE_DIR}/GUI/EditorObjectWindow" "${CMAKE_CURRENT_SOURCE_DIR}/GUI/GameObjectWindow"
                                                  "${CMAKE_CURRENT_SOURCE_DIR}/GUI/Levelwindow")
target_link_libraries (${MODULE_NAME} NANOGUI2 Engine)
target_compile_features(${MODULE_NAME} PUBLIC cxx_std_17)