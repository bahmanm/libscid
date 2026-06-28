cmake_minimum_required( VERSION 3.23 )

project(
    libscid
    LANGUAGES C CXX )

include( GNUInstallDirs )

set(
    SCID_PUBLIC_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/scid.h" )

set( SCID_LIBRARY_TYPE "" )
if( DEFINED LIBSCID_INSTALL AND LIBSCID_INSTALL )
    set( SCID_LIBRARY_TYPE SHARED )
endif()

add_library(
    scid
    ${SCID_LIBRARY_TYPE}
    src/scid.cpp )
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
    PRIVATE cxx_std_20 )

get_target_property( SCID_TARGET_TYPE scid TYPE )
if( SCID_TARGET_TYPE STREQUAL "SHARED_LIBRARY" )
    target_compile_definitions(
        scid
        PRIVATE SCID_BUILDING_LIBRARY
        PUBLIC SCID_SHARED )
endif()

target_include_directories(
    scid
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>" )

target_link_libraries(
    scid
    PRIVATE
        LibScidCpp::Core
        LibScidCpp::Database
        LibScidCpp::Eco )

if( PROJECT_IS_TOP_LEVEL )
    include( CTest )
endif()

if( BUILD_TESTING )
    add_executable(
        scid_tests_c
        tests/c/test_cursor.c
        tests/c/test_cursor_mutation.c
        tests/c/test_database.c
        tests/c/test_eco.c
        tests/c/test_game.c
        tests/c/test_libscid.c
        tests/c/test_movespec.c
        tests/c/test_nag.c
        tests/c/test_piece.c
        tests/c/test_position.c
        tests/c/test_pgn_contract.c
        tests/c/test_square.c )
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
