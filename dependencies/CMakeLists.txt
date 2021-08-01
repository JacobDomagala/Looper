# Function to download resource from git repo
function(FIND_AND_DOWNLOAD_GIT_RESOURCE ResourceName ResourceURL)
   if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${ResourceName})
      message("${ResourceName} not found!")
      execute_process(COMMAND git clone --recursive ${ResourceURL} ${CMAKE_CURRENT_SOURCE_DIR}/${ResourceName})
   else()
      message("${ResourceName} found!")
   endif()
endfunction()

# Function to download resource in zip format
function(FIND_AND_DOWNLOAD_ZIP_RESOURCE ResourceURL ResourceName Version)
   # download resource if zip file is missing
   if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${ResourceName}-${Version})
      message(STATUS "${ResourceName} needs to be downloaded!")

      # download
      file(DOWNLOAD ${ResourceURL} ${CMAKE_CURRENT_SOURCE_DIR}/${ResourceName}.zip SHOW_PROGRESS STATUS STAT LOG LG)

      # print status and log if file wasn't downloaded properly
      if(NOT("0" IN_LIST STAT))
         message("File status = ${STAT}")
         message("File LOG = ${LG}")
      endif()

      # unzip
      execute_process(WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMMAND ${CMAKE_COMMAND} -E tar xvf ${ResourceName}.zip)
   else()
      message(STATUS "${ResourceName} already downloaded")
   endif()
endfunction()

add_subdirectory(glad)
add_subdirectory(glfw)
add_subdirectory(imgui)
add_subdirectory(freetype)
add_subdirectory(stb_image)
add_subdirectory(glm)
add_subdirectory(json)
add_subdirectory(fmt)

