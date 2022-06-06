FIND_AND_DOWNLOAD_GIT_RESOURCE(stb_image https://github.com/nothings/stb.git)
add_library(stb_image INTERFACE IMPORTED GLOBAL)
target_include_directories(stb_image INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/stb_image")
