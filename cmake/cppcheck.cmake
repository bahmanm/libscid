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
    "warning"
    CACHE STRING "Comma-separated cppcheck check classes enabled by the cppcheck target." )

add_custom_target(
    cppcheck
    COMMAND
        "${LIBSCID_CPPCHECK}"
        "--project=${CMAKE_BINARY_DIR}/compile_commands.json"
        "--enable=${LIBSCID_CPPCHECK_ENABLE}"
        --error-exitcode=1
        "--exitcode-suppressions=${PROJECT_SOURCE_DIR}/cmake/cppcheck-exitcode-suppressions.txt"
        --inline-suppr
        --quiet
        --suppress=missingIncludeSystem
        --suppress=unmatchedSuppression
        "-i${CMAKE_BINARY_DIR}/_deps"
        "--template=gcc"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Running cppcheck static analysis." )
