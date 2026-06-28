cmake_minimum_required( VERSION 3.23 )

project(
    libscid-eco
    LANGUAGES CXX )

include( GNUInstallDirs )

set(
    SCID_ECO_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/code.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/book.cpp" )

set(
    SCID_ECO_PUBLIC_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/eco/code.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/scid/eco/book.h" )

add_library(
    scid_eco
    STATIC
    ${SCID_ECO_SOURCES} )
add_library(
    LibScidCpp::Eco
    ALIAS scid_eco )
target_sources(
    scid_eco
    PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
        FILES ${SCID_ECO_PUBLIC_HEADERS} )
target_compile_features(
    scid_eco
    PUBLIC cxx_std_20 )

set_target_properties(
    scid_eco
    PROPERTIES POSITION_INDEPENDENT_CODE ON )

target_include_directories(
    scid_eco
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>" )
target_link_libraries(
    scid_eco
    PUBLIC LibScidCpp::Core )

if( PROJECT_IS_TOP_LEVEL )
    include( CTest )
endif()

if( BUILD_TESTING )
    if( NOT TARGET gtest_main )
        include( FetchContent )
        set( INSTALL_GTEST OFF CACHE BOOL "Disable installation rules for googletest." FORCE )
        set( BUILD_GMOCK OFF CACHE BOOL "Do not build GoogleMock; ECO tests use GoogleTest only." FORCE )
        set( gtest_force_shared_crt ON CACHE BOOL "Always use msvcrt.dll" FORCE )

        fetchcontent_declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
            URL_HASH SHA256=65fab701d9829d38cb77c14acdc431d2108bfdbf8979e40eb8ae567edf10b27c )
        fetchcontent_makeavailable( googletest )
    endif()

    file(
        GLOB SCID_ECO_TEST_SOURCES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/cxx/*.cpp" )
    add_executable(
        scid_tests_eco_cpp
        ${SCID_ECO_TEST_SOURCES} )
    target_link_libraries(
        scid_tests_eco_cpp
        PRIVATE LibScidCpp::Eco gtest_main )
    libscid_enable_test_runtime_dlls( scid_tests_eco_cpp )
    add_executable(
        LibScidCpp::Tests::Eco::Cpp
        ALIAS scid_tests_eco_cpp )

    add_test(
        NAME eco_cpp_test
        COMMAND $<TARGET_FILE:LibScidCpp::Tests::Eco::Cpp> )
    set_tests_properties(
        eco_cpp_test
        PROPERTIES LABELS "cpp;eco" )
endif()
