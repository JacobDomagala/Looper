# Function to download resource from git repo
function(FIND_AND_DOWNLOAD_GIT_RESOURCE ResourceName ResourceURL)
   if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${ResourceName})
      message("${ResourceName} not found!")
      execute_process(COMMAND git clone --recursive ${ResourceURL} ${CMAKE_CURRENT_SOURCE_DIR}/${ResourceName})
   else()
      message("${ResourceName} found!")
   endif()
endfunction()

include(FindVulkan)
if (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
else()
	message(STATUS ${Vulkan_LIBRARY})
endif()

add_subdirectory(stb_image)

