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
    "${PROJECT_SOURCE_DIR}/libscid/*.c"
    "${PROJECT_SOURCE_DIR}/libscid/*.cc"
    "${PROJECT_SOURCE_DIR}/libscid/*.cpp"
    "${PROJECT_SOURCE_DIR}/libscid/*.cxx"
    "${PROJECT_SOURCE_DIR}/libscid/*.h"
    "${PROJECT_SOURCE_DIR}/libscid/*.hh"
    "${PROJECT_SOURCE_DIR}/libscid/*.hpp"
    "${PROJECT_SOURCE_DIR}/libscid/*.hxx"
    "${PROJECT_SOURCE_DIR}/libscid-cpp/*.c"
    "${PROJECT_SOURCE_DIR}/libscid-cpp/*.cc"
    "${PROJECT_SOURCE_DIR}/libscid-cpp/*.cpp"
    "${PROJECT_SOURCE_DIR}/libscid-cpp/*.cxx"
    "${PROJECT_SOURCE_DIR}/libscid-cpp/*.h"
    "${PROJECT_SOURCE_DIR}/libscid-cpp/*.hh"
    "${PROJECT_SOURCE_DIR}/libscid-cpp/*.hpp"
    "${PROJECT_SOURCE_DIR}/libscid-cpp/*.hxx" )

add_custom_target(
    format
    COMMAND "${LIBSCID_CLANG_FORMAT}" -i ${LIBSCID_FORMAT_SOURCES}
    COMMENT "Formatting C and C++ sources with clang-format." )

add_custom_target(
    format-check
    COMMAND "${LIBSCID_CLANG_FORMAT}" --dry-run --Werror ${LIBSCID_FORMAT_SOURCES}
    COMMENT "Checking C and C++ source formatting with clang-format." )
