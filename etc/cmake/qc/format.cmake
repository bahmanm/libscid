find_program(
    LIBSCID_CLANG_FORMAT
    NAMES clang-format-20 clang-format
    HINTS "$ENV{LLVM20}/bin" "/opt/homebrew/opt/llvm@20/bin" )

if( NOT LIBSCID_CLANG_FORMAT )
    add_custom_target(
        format
        COMMAND "${CMAKE_COMMAND}" -E echo "clang-format was not found."
        COMMAND "${CMAKE_COMMAND}" -E false )
    add_custom_target(
        format-check
        COMMAND "${CMAKE_COMMAND}" -E echo "clang-format was not found."
        COMMAND "${CMAKE_COMMAND}" -E false )
    return()
endif()

file(
    GLOB_RECURSE LIBSCID_FORMAT_SOURCES
    "${LIBSCID_SOURCE_ROOT}/examples/*.c"
    "${LIBSCID_SOURCE_ROOT}/examples/*.cc"
    "${LIBSCID_SOURCE_ROOT}/examples/*.cpp"
    "${LIBSCID_SOURCE_ROOT}/examples/*.cxx"
    "${LIBSCID_SOURCE_ROOT}/examples/*.h"
    "${LIBSCID_SOURCE_ROOT}/examples/*.hh"
    "${LIBSCID_SOURCE_ROOT}/examples/*.hpp"
    "${LIBSCID_SOURCE_ROOT}/examples/*.hxx"
    "${LIBSCID_SOURCE_ROOT}/capi/*.c"
    "${LIBSCID_SOURCE_ROOT}/capi/*.cc"
    "${LIBSCID_SOURCE_ROOT}/capi/*.cpp"
    "${LIBSCID_SOURCE_ROOT}/capi/*.cxx"
    "${LIBSCID_SOURCE_ROOT}/capi/*.h"
    "${LIBSCID_SOURCE_ROOT}/capi/*.hh"
    "${LIBSCID_SOURCE_ROOT}/capi/*.hpp"
    "${LIBSCID_SOURCE_ROOT}/capi/*.hxx"
    "${LIBSCID_SOURCE_ROOT}/internal/*.c"
    "${LIBSCID_SOURCE_ROOT}/internal/*.cc"
    "${LIBSCID_SOURCE_ROOT}/internal/*.cpp"
    "${LIBSCID_SOURCE_ROOT}/internal/*.cxx"
    "${LIBSCID_SOURCE_ROOT}/internal/*.h"
    "${LIBSCID_SOURCE_ROOT}/internal/*.hh"
    "${LIBSCID_SOURCE_ROOT}/internal/*.hpp"
    "${LIBSCID_SOURCE_ROOT}/internal/*.hxx" )

list(
    FILTER LIBSCID_FORMAT_SOURCES
    EXCLUDE REGEX
    "/(_build[^/]*|build|CMakeFiles)/" )

add_custom_target(
    format
    COMMAND "${LIBSCID_CLANG_FORMAT}" -i ${LIBSCID_FORMAT_SOURCES}
    COMMENT "Formatting C and C++ sources with clang-format." )

add_custom_target(
    format-check
    COMMAND "${LIBSCID_CLANG_FORMAT}" --dry-run --Werror ${LIBSCID_FORMAT_SOURCES}
    COMMENT "Checking C and C++ source formatting with clang-format." )
