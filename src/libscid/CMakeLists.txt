cmake_minimum_required( VERSION 3.28 )

if( DEFINED LIBSCID_VERSION AND LIBSCID_VERSION MATCHES "^[0-9]+(\\.[0-9]+)?(\\.[0-9]+)?(\\.[0-9]+)?$" )
    unset( LIBSCID_VERSION CACHE )
    unset( LIBSCID_VERSION )
endif()
set( LIBSCID_PROJECT_VERSION "0.0.0" CACHE STRING "Numeric libscid version used for CMake package compatibility." )

project(
    libscid
    VERSION "${LIBSCID_PROJECT_VERSION}"
    LANGUAGES C CXX )

include( "${LIBSCID_SOURCE_ROOT}/cmake/project.cmake" )

if( NOT TARGET LibScidInternal::Core )
    add_subdirectory( "${LIBSCID_SOURCE_ROOT}/src/internal" "${CMAKE_CURRENT_BINARY_DIR}/_deps/libscid-internal" )
endif()

set(
    SCID_PUBLIC_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/_platform.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/database.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/eco.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/game.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/movespec.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/position.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/primitives.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/scid.h" )

set( SCID_LIBRARY_TYPE "" )
if( DEFINED LIBSCID_INSTALL AND LIBSCID_INSTALL )
    set( SCID_LIBRARY_TYPE SHARED )
endif()

add_library(
    scid
    ${SCID_LIBRARY_TYPE}
    src/database.cpp
    src/eco.cpp
    src/game.cpp
    src/movespec.cpp
    src/position.cpp
    src/primitives.cpp
    src/support.cpp )
add_library(
    LibScid::LibScid
    ALIAS scid )

target_sources(
    scid
    PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
        FILES ${SCID_PUBLIC_HEADERS} )

target_compile_features(
    scid
    PRIVATE cxx_std_23 )

set_target_properties(
    scid
    PROPERTIES CXX_EXTENSIONS OFF )

get_target_property( SCID_TARGET_TYPE scid TYPE )
if( SCID_TARGET_TYPE STREQUAL "SHARED_LIBRARY" )
    target_compile_definitions(
        scid
        PRIVATE SCID_BUILDING_LIBRARY
        PUBLIC SCID_SHARED )
endif()

target_include_directories(
    scid
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/include"
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>" )

target_link_libraries(
    scid
    PRIVATE
        LibScidInternal::Core
        LibScidInternal::Database
        LibScidInternal::Eco )

if( PROJECT_IS_TOP_LEVEL )
    include( CTest )
endif()

if( BUILD_TESTING )
    add_executable(
        scid_tests_c
        "${LIBSCID_SOURCE_ROOT}/tests/libscid/c/test_cursor.c"
        "${LIBSCID_SOURCE_ROOT}/tests/libscid/c/test_cursor_mutation.c"
        "${LIBSCID_SOURCE_ROOT}/tests/libscid/c/test_database.c"
        "${LIBSCID_SOURCE_ROOT}/tests/libscid/c/test_eco.c"
        "${LIBSCID_SOURCE_ROOT}/tests/libscid/c/test_game.c"
        "${LIBSCID_SOURCE_ROOT}/tests/libscid/c/test_libscid.c"
        "${LIBSCID_SOURCE_ROOT}/tests/libscid/c/test_movespec.c"
        "${LIBSCID_SOURCE_ROOT}/tests/libscid/c/test_nag.c"
        "${LIBSCID_SOURCE_ROOT}/tests/libscid/c/test_piece.c"
        "${LIBSCID_SOURCE_ROOT}/tests/libscid/c/test_position.c"
        "${LIBSCID_SOURCE_ROOT}/tests/libscid/c/test_pgn_contract.c"
        "${LIBSCID_SOURCE_ROOT}/tests/libscid/c/test_square.c" )
    target_link_libraries(
        scid_tests_c
        PRIVATE LibScid::LibScid )
    set_target_properties(
        scid_tests_c
        PROPERTIES LINKER_LANGUAGE CXX )
    libscid_enable_test_runtime_dlls( scid_tests_c )

    add_test(
        NAME libscid_c_test
        COMMAND scid_tests_c )
    set_tests_properties(
        libscid_c_test
        PROPERTIES LABELS "c;libscid" )
endif()

if( LIBSCID_BUILD_DOCS )
    include( "${LIBSCID_SOURCE_ROOT}/cmake/docs.cmake" )
endif()

if( LIBSCID_INSTALL )
    include( "${CMAKE_CURRENT_SOURCE_DIR}/cmake/install/main.cmake" )
    include( "${CMAKE_CURRENT_SOURCE_DIR}/cmake/package/cpack.cmake" )
endif()
