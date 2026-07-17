file( STRINGS "${LIBSCID_INCLUDE_CLEANER_SOURCES_FILE}" LIBSCID_INCLUDE_CLEANER_SOURCES )

foreach( source IN LISTS LIBSCID_INCLUDE_CLEANER_SOURCES )
    message( STATUS "include-cleaner: ${source}" )
    execute_process(
        COMMAND
            "${LIBSCID_CLANG_INCLUDE_CLEANER}"
            -p "${LIBSCID_INCLUDE_CLEANER_BUILD_DIR}"
            --remove
            --print=changes
            "--ignore-headers=${LIBSCID_INCLUDE_CLEANER_IGNORE_HEADERS}"
            "${source}"
        WORKING_DIRECTORY "${LIBSCID_INCLUDE_CLEANER_WORKING_DIR}"
        RESULT_VARIABLE _libscid_include_cleaner_result )

    if( NOT _libscid_include_cleaner_result EQUAL 0 )
        message( FATAL_ERROR "include-cleaner failed for ${source}." )
    endif()
endforeach()
