function(compile_shader)
    set(options "")
    set(oneValueArgs SOURCE_FILE OUTPUT_FILE_NAME)
    set(multiValueArgs DEFINES DEPENDS)
    cmake_parse_arguments(params "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT params_SOURCE_FILE)
        message(FATAL_ERROR "compile_shader: SOURCE_FILE argument missing")
    endif()

    if (NOT params_OUTPUT_FILE_NAME)
        message(FATAL_ERROR "compile_shader: OUTPUT_FILE_NAME argument missing")
    endif()

    set(define_flags)
    foreach(define IN LISTS params_DEFINES)
        list(APPEND define_flags "-D${define}")
    endforeach()
    get_filename_component(output_directory "${params_OUTPUT_FILE_NAME}" DIRECTORY)
    get_filename_component(output_name "${params_OUTPUT_FILE_NAME}" NAME)
    string(MAKE_C_IDENTIFIER "shader_${output_name}" shader_target)
    add_custom_command(
        OUTPUT "${params_OUTPUT_FILE_NAME}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${output_directory}"
        COMMAND "${Vulkan_GLSLC_EXECUTABLE}" ${define_flags} "${params_SOURCE_FILE}"
                -o "${params_OUTPUT_FILE_NAME}"
        DEPENDS "${params_SOURCE_FILE}" ${params_DEPENDS}
        COMMENT "Compiling ${output_name}"
        VERBATIM)
    add_custom_target(${shader_target} DEPENDS "${params_OUTPUT_FILE_NAME}")
    add_dependencies(Engine ${shader_target})

endfunction()
