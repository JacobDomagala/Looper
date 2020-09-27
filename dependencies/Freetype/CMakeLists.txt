cmake_minimum_required(VERSION 3.13)

set(FREETYPE_VERSION 2.10.2)

FIND_AND_DOWNLOAD_ZIP_RESOURCE(https://sourceforge.net/projects/freetype/files/latest/download FREETYPE ${FREETYPE_VERSION})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/freetype-${FREETYPE_VERSION})