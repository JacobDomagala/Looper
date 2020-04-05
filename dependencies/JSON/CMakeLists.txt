cmake_minimum_required(VERSION 3.13)

#JSON
FIND_AND_DOWNLOAD_GIT_RESOURCE(JSON https://github.com/nlohmann/json.git)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/JSON)

