cmake_minimum_required(VERSION 3.13)

file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/glad/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/include/KHR/*.h")
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_library(GLAD INTERFACE IMPORTED GLOBAL )
target_sources(GLAD INTERFACE ${HEADERS} ${SOURCES})
target_include_directories(GLAD INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")
# For linux we may need to add -ldl
#target_link_libraries (GLAD INTERFACE opengl32)