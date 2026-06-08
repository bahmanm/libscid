function( libscid_enable_test_runtime_dlls target )
    if( WIN32 AND BUILD_SHARED_LIBS )
        add_custom_command(
            TARGET "${target}"
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "$<TARGET_RUNTIME_DLLS:${target}>"
                "$<TARGET_FILE_DIR:${target}>"
            COMMAND_EXPAND_LISTS )
    endif()
endfunction()
