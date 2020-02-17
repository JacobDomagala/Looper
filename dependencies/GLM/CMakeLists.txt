cmake_minimum_required(VERSION 3.13)

FIND_AND_DOWNLOAD_ZIP_RESOURCE(https://github.com/g-truc/glm/archive/${GLM_VERSION}.zip GLM ${GLM_VERSION})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glm-${GLM_VERSION})