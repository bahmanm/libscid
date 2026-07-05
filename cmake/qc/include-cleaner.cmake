find_program(
    LIBSCID_CLANG_INCLUDE_CLEANER
    NAMES clang-include-cleaner-20 clang-include-cleaner
    HINTS "$ENV{LLVM20}/bin" "/opt/homebrew/opt/llvm@20/bin" )

if( NOT LIBSCID_CLANG_INCLUDE_CLEANER )
    add_custom_target(
        include-cleaner
        COMMAND "${CMAKE_COMMAND}" -E echo "clang-include-cleaner was not found."
        COMMAND "${CMAKE_COMMAND}" -E false )
    return()
endif()

file(
    GLOB LIBSCID_INCLUDE_CLEANER_SOURCES
    CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/libscid/src/*.cpp" )

set(
    LIBSCID_INCLUDE_CLEANER_IGNORE_HEADERS
    ""
    CACHE STRING "Comma-separated header suffix regexes ignored by the include-cleaner target." )

set( LIBSCID_INCLUDE_CLEANER_SOURCES_FILE "${CMAKE_BINARY_DIR}/include-cleaner-sources.txt" )
file( WRITE "${LIBSCID_INCLUDE_CLEANER_SOURCES_FILE}" "" )
foreach( source IN LISTS LIBSCID_INCLUDE_CLEANER_SOURCES )
    file( APPEND "${LIBSCID_INCLUDE_CLEANER_SOURCES_FILE}" "${source}\n" )
endforeach()

add_custom_target(
    include-cleaner
    COMMAND
        "${CMAKE_COMMAND}"
        "-DLIBSCID_CLANG_INCLUDE_CLEANER=${LIBSCID_CLANG_INCLUDE_CLEANER}"
        "-DLIBSCID_INCLUDE_CLEANER_BUILD_DIR=${CMAKE_BINARY_DIR}"
        "-DLIBSCID_INCLUDE_CLEANER_IGNORE_HEADERS=${LIBSCID_INCLUDE_CLEANER_IGNORE_HEADERS}"
        "-DLIBSCID_INCLUDE_CLEANER_SOURCES_FILE=${LIBSCID_INCLUDE_CLEANER_SOURCES_FILE}"
        "-DLIBSCID_INCLUDE_CLEANER_WORKING_DIR=${PROJECT_SOURCE_DIR}"
        -P "${PROJECT_SOURCE_DIR}/cmake/qc/include-cleaner-run.cmake"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMENT "Printing include-cleaner recommendations for libscid implementation files."
    VERBATIM )
