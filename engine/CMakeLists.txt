set(MODULE_NAME Engine)
project(${MODULE_NAME})

file(GLOB_RECURSE HEADERS "*.hpp")
file(GLOB_RECURSE SOURCES "*.cpp")

add_library(${MODULE_NAME} STATIC ${SOURCES} ${HEADERS})
target_include_directories(${MODULE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/common"
                                          "${CMAKE_CURRENT_SOURCE_DIR}/core" "${CMAKE_CURRENT_SOURCE_DIR}/game"
                                          "${CMAKE_CURRENT_SOURCE_DIR}/input" "${CMAKE_CURRENT_SOURCE_DIR}/logger"
                                          "${CMAKE_CURRENT_SOURCE_DIR}/renderer"
                                          "${CMAKE_CURRENT_SOURCE_DIR}/scene" "${CMAKE_CURRENT_SOURCE_DIR}/utils"
                                          "${CMAKE_CURRENT_SOURCE_DIR}/time" "${CMAKE_CURRENT_SOURCE_DIR}/window")



# Load all TPLs
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)
find_package(Vulkan REQUIRED)

target_link_libraries_system(${MODULE_NAME} PUBLIC stb_image CONAN_PKG::fmt CONAN_PKG::glfw CONAN_PKG::imgui CONAN_PKG::glm CONAN_PKG::nlohmann_json)
target_link_libraries(${MODULE_NAME} PUBLIC project_warnings project_options Vulkan::Vulkan)
target_compile_features(${MODULE_NAME} PRIVATE cxx_std_20)

if(WIN32)
    target_compile_options(${MODULE_NAME} INTERFACE -DVK_USE_PLATFORM_WIN32_KHR)
endif()

option(UNITY_BUILD "Build with CMake Unity build" OFF)
if(UNITY_BUILD)
    message(STATUS "Building using CMake's Unity build")
    set_target_properties(${MODULE_NAME} PROPERTIES UNITY_BUILD ON)
endif()
