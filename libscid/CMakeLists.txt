cmake_minimum_required( VERSION 3.23 )

project(
    libscid
    LANGUAGES C CXX )

include( GNUInstallDirs )

set(
    SCID_PUBLIC_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/scid.h" )

add_library(
    scid
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

target_include_directories(
    scid
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>" )

target_link_libraries(
    scid
    PRIVATE LibScidCpp::Core )

if( PROJECT_IS_TOP_LEVEL )
    include( CTest )
endif()

if( BUILD_TESTING )
    add_executable(
        scid_tests_c
        tests/c/scid_nag_test.c )
    target_link_libraries(
        scid_tests_c
        PRIVATE LibScid::LibScid )
    set_target_properties(
        scid_tests_c
        PROPERTIES LINKER_LANGUAGE CXX )

    add_test(
        NAME libscid_c_test
        COMMAND scid_tests_c )
    set_tests_properties(
        libscid_c_test
        PROPERTIES LABELS "c;libscid" )
endif()
