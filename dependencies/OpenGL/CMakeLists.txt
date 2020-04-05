cmake_minimum_required(VERSION 3.13)

set(OPENGL_VERSION 4.5)
set(GLEW_VERSION 2.1.0)

find_package(OpenGL ${OPENGL_VERSION} EXACT REQUIRED)
set_target_properties(OpenGL::GL PROPERTIES IMPORTED_GLOBAL TRUE)

add_library(GLEW::GLEW INTERFACE IMPORTED GLOBAL)

if(WIN32)
   FIND_AND_DOWNLOAD_ZIP_RESOURCE(https://sourceforge.net/projects/glew/files/glew/${GLEW_VERSION}/glew-${GLEW_VERSION}-win32.zip GLEW ${GLEW_VERSION})
   target_link_directories(GLEW::GLEW INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/glew-${GLEW_VERSION}/lib/Release/x64")
   file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/glew-${GLEW_VERSION}/bin/Release/x64/glew32.dll" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
   file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/glew-${GLEW_VERSION}/bin/Release/x64/glew32.dll" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
   target_link_libraries(GLEW::GLEW INTERFACE "glew32")
else()
   FIND_AND_DOWNLOAD_ZIP_RESOURCE(https://sourceforge.net/projects/glew/files/glew/${GLEW_VERSION}/glew-${GLEW_VERSION}.tgz GLEW ${GLEW_VERSION})
endif()

target_include_directories(GLEW::GLEW INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/GLEW-${GLEW_VERSION}/include")
