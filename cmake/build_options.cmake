
option(UNITY_BUILD "Building with Unity/Jumbo mode enabled" ON)

function(add_build_options target_name)
    if(UNITY_BUILD)
        set_target_properties(${target_name} PROPERTIES UNITY_BUILD ON)
    endif()
endfunction(add_build_options)


