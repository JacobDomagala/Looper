cmake_minimum_required(VERSION 3.5)
project(DGame VERSION 1.0.0 LANGUAGES CXX)

set(RESOURCES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../resource)
set(CMAKE_DEBUG_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Debug/)
set(CMAKE_RELEASE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release/)

# Function to download resource from git repo
function(FIND_AND_DOWNLOAD_GIT_RESOURCE ResourceName ResourceURL)
   find_path(${ResourceName}_PATH ${ResourceName} ${RESOURCES_PATH})

   if(NOT ${ResourceName}_PATH)
      message("${ResourceName} not found!")
      execute_process(COMMAND git clone ${ResourceURL} ${RESOURCES_PATH}/${ResourceName})
   else()
      message("${ResourceName} found!")
   endif()
endfunction()

# Function to download resource in zip format
function(FIND_AND_DOWNLOAD_RESOURCE ResourceName ResourceURL)
   find_path(${ResourceName}_PATH ${ResourceName} ${RESOURCES_PATH})

   if(NOT ${ResourceName}_PATH)
      message("${ResourceName} not found!")
      file(DOWNLOAD ${ResourceURL} ${RESOURCES_PATH}/${ResourceName}/${ResourceName}.zip SHOW_PROGRESS)
      execute_process(COMMAND unzip ${RESOURCES_PATH}/${ResourceName}/${ResourceName} -d ${RESOURCES_PATH}/${ResourceName})
   else()
      message("${ResourceName} found!")
   endif()
endfunction()

# Function to copy dll files to build directory
function(COPY_LIBS_TO_BUILD_DIR BuildDirectory)
   find_path(OUTPUT_EXISTS ${BuildDirectory} ${CMAKE_BINARY_DIR})
   if(NOT ${OUTPUT_EXISTS})
      execute_process(COMMAND mkdir -p ${BuildDirectory})
      execute_process(COMMAND cp ${RESOURCES_PATH}/GLEW/glew-2.1.0/bin/Release/x64/glew32.dll ${BuildDirectory})
      execute_process(COMMAND cp ${RESOURCES_PATH}/FREETYPE/win64/freetype.dll ${BuildDirectory})
   endif()
endfunction()

FIND_AND_DOWNLOAD_GIT_RESOURCE(FREETYPE https://github.com/ubawurinna/freetype-windows-binaries.git)
FIND_AND_DOWNLOAD_GIT_RESOURCE(STB_IMAGE https://github.com/nothings/stb.git)
FIND_AND_DOWNLOAD_RESOURCE(GLEW https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0-win32.zip)
FIND_AND_DOWNLOAD_RESOURCE(GLM https://github.com/g-truc/glm/releases/download/0.9.8.5/glm-0.9.8.5.zip)

set(RESOURCERS_INCLUDES ${RESOURCES_PATH}/GLM/glm/glm 
                        ${RESOURCES_PATH}/GLEW/glew-2.1.0/include/GL 
                        ${RESOURCES_PATH}/STB_IMAGE
                        ${RESOURCES_PATH}/FREETYPE/include)

set(RESOURCES_LIBS ${RESOURCES_PATH}/GLEW/glew-2.1.0/lib/Release/x64 
                   ${RESOURCES_PATH}/FREETYPE/win64)

COPY_LIBS_TO_BUILD_DIR(${CMAKE_DEBUG_RUNTIME_OUTPUT_DIRECTORY})
COPY_LIBS_TO_BUILD_DIR(${CMAKE_RELEASE_RUNTIME_OUTPUT_DIRECTORY})

file(GLOB HEADERS "*.hpp")
file(GLOB SOURCES "*.cpp")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${RESOURCERS_INCLUDES})
link_directories(${RESOURCES_LIBS})

if(MSVC)
    add_compile_options("/W4" "$<$<CONFIG:RELEASE>:/O2>")
else()
    add_compile_options("-Wall" "-Wextra" "-Werror" "$<$<CONFIG:RELEASE>:-O3>")
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options("-stdlib=libc++")
    else()
        # nothing special for gcc at the moment
    endif()
endif()

add_executable(DGame WIN32 ${HEADERS} ${SOURCES})
target_link_libraries (DGame glew32 glew32s opengl32 freetype)