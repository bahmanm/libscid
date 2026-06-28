cmake_minimum_required( VERSION 3.23 )

project(
    libscid-core
    LANGUAGES CXX )

include( GNUInstallDirs )

set(
    SCID_CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/game_cursor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/move.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/movetext_cursor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/movetext_location.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/nags.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/notation.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pgn_decode.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/pgn_traversal.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/game.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/position.cpp" )

set(
    SCID_CORE_PUBLIC_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/attacks.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/board.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/date.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/dstring.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/error.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/fullmove.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/game.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/game_cursor.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/game_positions.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/movetext_cursor.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/movetext_location.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/game_result.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/hash.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/move.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/movelist.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/move_predicates.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/nags.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/notation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/pgn/decode.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/pgn/encode.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/pgn/traversal.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/position.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/primitives.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/rating.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/square_collections.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/core/square_moves.h" )

add_library(
    scid_core
    STATIC
    ${SCID_CORE_SOURCES} )
add_library(
    LibScidCpp::Core
    ALIAS scid_core )
target_sources(
    scid_core
    PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
        FILES ${SCID_CORE_PUBLIC_HEADERS} )
target_compile_features(
    scid_core
    PUBLIC cxx_std_20 )

set_target_properties(
    scid_core
    PROPERTIES POSITION_INDEPENDENT_CODE ON )

target_include_directories(
    scid_core
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/internal" )

if( PROJECT_IS_TOP_LEVEL )
    include( CTest )
endif()

if( BUILD_TESTING )
    if( NOT TARGET gtest_main )
        include( FetchContent )
        set( INSTALL_GTEST OFF CACHE BOOL "Disable installation rules for googletest." FORCE )
        set( BUILD_GMOCK OFF CACHE BOOL "Do not build GoogleMock; core tests use GoogleTest only." FORCE )
        set( gtest_force_shared_crt ON CACHE BOOL "Always use msvcrt.dll" FORCE )

        fetchcontent_declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
            URL_HASH SHA256=65fab701d9829d38cb77c14acdc431d2108bfdbf8979e40eb8ae567edf10b27c )
        fetchcontent_makeavailable( googletest )
    endif()

    file(
        GLOB SCID_CORE_TEST_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/cxx/*.cpp" )
    add_executable(
        scid_tests_core_cpp
        ${SCID_CORE_TEST_SOURCES} )
    target_include_directories(
        scid_tests_core_cpp
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/internal" )
    target_link_libraries(
        scid_tests_core_cpp
        PRIVATE LibScidCpp::Core gtest_main )
    libscid_enable_test_runtime_dlls( scid_tests_core_cpp )
    add_executable(
        LibScidCpp::Tests::Core::Cpp
        ALIAS scid_tests_core_cpp )

    add_test(
        NAME core_cpp_test
        COMMAND $<TARGET_FILE:LibScidCpp::Tests::Core::Cpp> )
    set_tests_properties(
        core_cpp_test
        PROPERTIES LABELS "cpp;core" )
endif()
