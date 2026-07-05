find_program(
    LIBSCID_CPPCHECK
    NAMES cppcheck )

if( NOT LIBSCID_CPPCHECK )
    add_custom_target(
        cppcheck
        COMMAND "${CMAKE_COMMAND}" -E echo "cppcheck was not found."
        COMMAND "${CMAKE_COMMAND}" -E false )
    return()
endif()

set(
    LIBSCID_CPPCHECK_ENABLE
    "warning,portability"
    CACHE STRING "Comma-separated cppcheck check classes enabled by the cppcheck target." )

set( LIBSCID_CPPCHECK_EXITCODE_SUPPRESSIONS
     "${CMAKE_BINARY_DIR}/cppcheck-exitcode-suppressions.txt" )
file( WRITE "${LIBSCID_CPPCHECK_EXITCODE_SUPPRESSIONS}" "" )

add_custom_target(
    cppcheck
    COMMAND
        "${LIBSCID_CPPCHECK}"
        "--project=${CMAKE_BINARY_DIR}/compile_commands.json"
        "--enable=${LIBSCID_CPPCHECK_ENABLE}"
        --error-exitcode=1
        "--exitcode-suppressions=${LIBSCID_CPPCHECK_EXITCODE_SUPPRESSIONS}"
        --inline-suppr
        --quiet
        --suppress=missingIncludeSystem
        --suppress=unmatchedSuppression
        "-i${CMAKE_BINARY_DIR}/_deps"
        "--template=gcc"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Running cppcheck static analysis." )
