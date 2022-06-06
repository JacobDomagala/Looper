set(GLM_VERSION 0.9.9.8)

FIND_AND_DOWNLOAD_ZIP_RESOURCE(https://github.com/g-truc/glm/archive/${GLM_VERSION}.zip glm ${GLM_VERSION})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glm-${GLM_VERSION})
