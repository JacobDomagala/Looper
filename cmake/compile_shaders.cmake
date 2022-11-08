function(compile_shader)
    set(options "")
    set(oneValueArgs SOURCE_FILE OUTPUT_FILE_NAME)
    set(multiValueArgs DEFINES)
    cmake_parse_arguments(params "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT params_SOURCE_FILE)
        message(FATAL_ERROR "compile_shader: SOURCE_FILE argument missing")
    endif()

    if (NOT params_OUTPUT_FILE_NAME)
        message(FATAL_ERROR "compile_shader: OUTPUT_FILE_NAME argument missing")
    endif()

    execute_process(COMMAND ${Vulkan_GLSLC_EXECUTABLE} "${params_SOURCE_FILE}" -o "${params_OUTPUT_FILE_NAME}")

endfunction()
