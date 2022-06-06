file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/glad/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/include/KHR/*.h")
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_library(glad STATIC ${HEADERS} ${SOURCES} )
#target_sources(GLAD PUBLIC )
target_include_directories(glad SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
# For linux we may need to add -ldl
#target_link_libraries (GLAD INTERFACE opengl32)
