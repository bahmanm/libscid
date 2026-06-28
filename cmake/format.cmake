find_program(
    LIBSCID_CLANG_FORMAT
    NAMES clang-format-20 clang-format )

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
    CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/examples/*.c"
    "${PROJECT_SOURCE_DIR}/examples/*.cc"
    "${PROJECT_SOURCE_DIR}/examples/*.cpp"
    "${PROJECT_SOURCE_DIR}/examples/*.cxx"
    "${PROJECT_SOURCE_DIR}/examples/*.h"
    "${PROJECT_SOURCE_DIR}/examples/*.hh"
    "${PROJECT_SOURCE_DIR}/examples/*.hpp"
    "${PROJECT_SOURCE_DIR}/examples/*.hxx"
    "${PROJECT_SOURCE_DIR}/src/*.c"
    "${PROJECT_SOURCE_DIR}/src/*.cc"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.cxx"
    "${PROJECT_SOURCE_DIR}/src/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.hh"
    "${PROJECT_SOURCE_DIR}/src/*.hpp"
    "${PROJECT_SOURCE_DIR}/src/*.hxx"
    "${PROJECT_SOURCE_DIR}/tests/*.c"
    "${PROJECT_SOURCE_DIR}/tests/*.cc"
    "${PROJECT_SOURCE_DIR}/tests/*.cpp"
    "${PROJECT_SOURCE_DIR}/tests/*.cxx"
    "${PROJECT_SOURCE_DIR}/tests/*.h"
    "${PROJECT_SOURCE_DIR}/tests/*.hh"
    "${PROJECT_SOURCE_DIR}/tests/*.hpp"
    "${PROJECT_SOURCE_DIR}/tests/*.hxx" )

add_custom_target(
    format
    COMMAND "${LIBSCID_CLANG_FORMAT}" -i ${LIBSCID_FORMAT_SOURCES}
    COMMENT "Formatting C and C++ sources with clang-format." )

add_custom_target(
    format-check
    COMMAND "${LIBSCID_CLANG_FORMAT}" --dry-run --Werror ${LIBSCID_FORMAT_SOURCES}
    COMMENT "Checking C and C++ source formatting with clang-format." )
