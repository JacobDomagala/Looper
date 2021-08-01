FIND_AND_DOWNLOAD_GIT_RESOURCE(json https://github.com/nlohmann/json.git)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/json)

