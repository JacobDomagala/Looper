cmake_minimum_required(VERSION 3.13)

#SDL2
FIND_AND_DOWNLOAD_ZIP_RESOURCE(https://www.libsdl.org/release/SDL2-${SDL_VERSION}.zip SDL2 ${SDL_VERSION})
#FIND_AND_DOWNLOAD_ZIP_RESOURCE(https://www.libsdl.org/release/SDL2-${SDL_VERSION}-win32-x64.zip SDL2_LIB ${SDL_VERSION})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SDL2-${SDL_VERSION})
#add_library(SDL2 INTERFACE IMPORTED GLOBAL)
#target_link_directories(SDL2 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
#target_link_libraries(SDL2 INTERFACE "SDL2")
#target_include_directories(SDL2 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/SDL2-${SDL_VERSION}/include")


#SDL2_Image
#FIND_AND_DOWNLOAD_ZIP_RESOURCE(https://www.libsdl.org/projects/SDL_image/release/SDL2_image-${SDL_IMAGE_VERSION}.zip SDL2_IMAGE ${SDL_IMAGE_VERSION})
#FIND_AND_DOWNLOAD_ZIP_RESOURCE(https://www.libsdl.org/projects/SDL_image/release/SDL2_image-${SDL_IMAGE_VERSION}-win32-x64.zip SDL2_IMAGE_LIB ${SDL_IMAGE_VERSION})

#add_library(SDL2_Image INTERFACE IMPORTED GLOBAL)
#target_link_directories(SDL2_Image INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
#target_link_libraries(SDL2_Image INTERFACE "SDL2_image")
#target_include_directories(SDL2_Image INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/SDL2_image-${SDL_IMAGE_VERSION}")


#SDL2_TTF
#FIND_AND_DOWNLOAD_ZIP_RESOURCE(https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-${SDL_TTF_VERSION}.zip SDL2_TTF ${SDL_TTF_VERSION})
#FIND_AND_DOWNLOAD_ZIP_RESOURCE(https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-${SDL_TTF_VERSION}-win32-x64.zip SDL2_TTF_LIB ${SDL_TTF_VERSION})

#add_library(SDL2_TTF INTERFACE IMPORTED GLOBAL)
#target_link_directories(SDL2_TTF INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
#target_link_libraries(SDL2_TTF INTERFACE "SDL2_ttf")
#target_include_directories(SDL2_TTF INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/SDL2_ttf-${SDL_TTF_VERSION}")


