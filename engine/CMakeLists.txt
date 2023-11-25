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
find_package(Vulkan REQUIRED)
find_package(fmt REQUIRED)
find_package(glfw3 REQUIRED)
find_package(imgui REQUIRED)
find_package(glm REQUIRED)
find_package(nlohmann_json REQUIRED)

target_link_libraries_system(${MODULE_NAME} PUBLIC stb_image fmt::fmt glfw imgui::imgui glm::glm nlohmann_json::nlohmann_json)
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
