find_program(
    LIBSCID_RUN_CLANG_TIDY
    NAMES run-clang-tidy-20 run-clang-tidy
    HINTS "$ENV{LLVM20}/bin" "/opt/homebrew/opt/llvm@20/bin" )

find_program(
    LIBSCID_CLANG_TIDY
    NAMES clang-tidy-20 clang-tidy
    HINTS "$ENV{LLVM20}/bin" "/opt/homebrew/opt/llvm@20/bin" )

if( NOT LIBSCID_RUN_CLANG_TIDY OR NOT LIBSCID_CLANG_TIDY )
    add_custom_target(
        clang-tidy
        COMMAND "${CMAKE_COMMAND}" -E echo "clang-tidy was not found."
        COMMAND "${CMAKE_COMMAND}" -E false )
    return()
endif()

get_filename_component( LIBSCID_CLANG_TIDY_BINDIR "${LIBSCID_CLANG_TIDY}" DIRECTORY )

if( WIN32 )
    set( LIBSCID_PATH_SEPARATOR "\;" )
else()
    set( LIBSCID_PATH_SEPARATOR ":" )
endif()

add_custom_target(
    clang-tidy
    COMMAND
        "${CMAKE_COMMAND}" -E env
        "PATH=${LIBSCID_CLANG_TIDY_BINDIR}${LIBSCID_PATH_SEPARATOR}$ENV{PATH}"
        "${LIBSCID_RUN_CLANG_TIDY}"
        -p "${CMAKE_BINARY_DIR}"
        -quiet
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Running clang-tidy static analysis." )
